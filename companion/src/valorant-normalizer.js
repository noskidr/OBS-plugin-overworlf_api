'use strict';

/*
 * GamePulse Companion — Valorant event normalizer.
 *
 * Turns raw Overwolf GEP Valorant events/info-updates into GamePulse protocol
 * events ({name,label,detail,importance,ts,...}) with human detail pulled from
 * kill_feed (weapon, victim, headshot). Multikill/ace are NOT derived here —
 * the OBS plugin's rules engine derives those from the kill stream, so both
 * the plugin path and a future direct path stay consistent.
 *
 * GEP Valorant schema (verified against dev.overwolf.com + insights.gg mining):
 *   feature "kill":       key kill|assist|headshot (running totals as event data)
 *   feature "death":      key death
 *   feature "match_info": keys kill_feed, match_start, match_end, spike_defused,
 *                         spike_detonated, planted_location, round_phase (info),
 *                         round_number (info), score (info)
 *
 * Weapon/agent dictionaries mirror insights.gg's VALORANT_WEAPON_DICT so the
 * on-stream detail reads naturally ("Vandal → Reyna (HS)").
 */

const IMP = { DEBUG: 0, MINOR: 1, NOTABLE: 2, EPIC: 3 };

// TX_Hud_* weapon texture id -> display name (subset; unknown falls back to a
// cleaned id).
const WEAPON_DICT = {
  TX_Hud_Pistol_Classic: 'Classic',
  TX_Hud_Pistol_Slim: 'Shorty',
  TX_Hud_Pistol_Boom: 'Frenzy',
  TX_Hud_Pistol_AutoPistol: 'Ghost',
  TX_Hud_Pistol_Luger: 'Sheriff',
  TX_Hud_SMG_Vector: 'Stinger',
  TX_Hud_SMG_Ninja: 'Spectre',
  TX_Hud_SG_Bucky: 'Bucky',
  TX_Hud_SG_Punch: 'Judge',
  TX_Hud_AR_Burst: 'Bulldog',
  TX_Hud_AR_Ghost: 'Guardian',
  TX_Hud_AR_Standard: 'Phantom',
  TX_Hud_AR_Vandal: 'Vandal',
  TX_Hud_SR_Leveraction: 'Marshal',
  TX_Hud_SR_Bolt: 'Outlaw',
  TX_Hud_SR_Sniper: 'Operator',
  TX_Hud_LMG_Ares: 'Ares',
  TX_Hud_LMG_HMG: 'Odin',
  TX_Hud_Melee_Standard: 'Melee',
};

function prettyWeapon(tex) {
  if (!tex) return '';
  if (WEAPON_DICT[tex]) return WEAPON_DICT[tex];
  // strip TX_Hud_<Class>_ prefix and Killfeed variants
  let s = String(tex).replace(/^TX_(Hud|Killfeed)_/i, '').replace(/_/g, ' ');
  return s.trim();
}

class ValorantNormalizer {
  constructor() {
    this.reset();
  }

  reset() {
    this.lastRoundPhase = '';
    this.roundNumber = 0;
    this.localPlayer = ''; // "Name#TAG" from GEP me/player_name
  }

  // Returns an array of protocol event objects (may be empty).
  handleEvent(evt, ts) {
    const out = [];
    const key = evt.key;
    const value = evt.value;

    switch (key) {
      case 'kill':
        // The kill counter fires only for the local player, but carries no
        // detail. Prefer kill_feed (which has weapon/victim/headshot) as the
        // canonical kill when we can identify the local player; fall back to
        // the counter otherwise so kills are never lost.
        if (!this.localPlayer) out.push(this._mk('kill', 'Kill', '', IMP.NOTABLE, ts));
        break;
      case 'assist':
        out.push(this._mk('assist', 'Assist', '', IMP.MINOR, ts));
        break;
      case 'headshot':
        // headshot counter ticks; treated as a flavor event (low importance)
        out.push(this._mk('headshot', 'Headshot', '', IMP.NOTABLE, ts));
        break;
      case 'death':
        out.push(this._mk('death', 'Death', '', IMP.MINOR, ts));
        break;
      case 'match_start':
        out.push(this._mk('match_start', 'Match Start', '', IMP.MINOR, ts));
        break;
      case 'match_end':
        out.push(this._mk('match_end', 'Match End', '', IMP.NOTABLE, ts));
        break;
      case 'spike_defused':
        out.push(this._mk('spike_defused', 'Spike Defused', '', IMP.EPIC, ts));
        break;
      case 'spike_detonated':
        out.push(this._mk('spike_detonated', 'Spike Detonated', '', IMP.NOTABLE, ts));
        break;
      case 'planted_location':
        // arrives at round end if spike was planted; use as a plant marker
        out.push(this._mk('spike_planted', 'Spike Planted', `Site ${value}`, IMP.NOTABLE, ts));
        break;
      case 'kill_feed':
        out.push(this._killFeed(value, ts));
        break;
      default:
        break;
    }
    return out.filter(Boolean);
  }

  handleInfo(info, ts) {
    const out = [];
    if (info.key === 'player_name' && info.value) {
      this.localPlayer = String(info.value); // "Name#TAG"
      return out;
    }
    if (info.key === 'round_phase') {
      const phase = String(info.value);
      if (phase !== this.lastRoundPhase) {
        this.lastRoundPhase = phase;
        // Emit as info so the plugin can synthesize round_start/round_end.
        out.push({
          t: 'info',
          game: { id: 21640, name: 'VALORANT' },
          feature: 'match_info',
          category: 'match_info',
          key: 'round_phase',
          value: phase,
          ts,
        });
      }
    } else if (info.key === 'round_number') {
      this.roundNumber = parseInt(info.value, 10) || this.roundNumber;
    }
    return out;
  }

  _killFeed(value, ts) {
    // value: { attacker, victim, is_attacker_teammate, is_victim_teammate,
    //          weapon, headshot, ... } (already JSON-decoded by GepService).
    // The kill feed contains EVERY player's kills, not just yours — only the
    // lines where the local player is the attacker are the streamer's kills.
    if (!value || typeof value !== 'object') return null;

    // Require a known local player so we don't double-count with the kill
    // counter fallback (which fires while localPlayer is unknown).
    if (!this.localPlayer) return null;
    if ((value.attacker || '') !== this.localPlayer) return null; // someone else's kill

    const weapon = prettyWeapon(value.weapon);
    const victim = value.victim || '';
    const hs = value.headshot ? ' (HS)' : '';
    const detail = ([weapon, victim ? '→ ' + victim : ''].filter(Boolean).join(' ') + hs).trim();

    // Emit as the canonical "kill" so the plugin's rules engine counts it for
    // multikill/ace, now carrying weapon/victim/headshot detail. Headshots are
    // notable.
    return this._mk('kill', 'Kill', detail, value.headshot ? IMP.NOTABLE : IMP.NOTABLE, ts);
  }

  _mk(name, label, detail, importance, ts) {
    return {
      t: 'event',
      game: { id: 21640, name: 'VALORANT' },
      name,
      label,
      detail,
      importance,
      ts,
    };
  }
}

module.exports = { ValorantNormalizer, prettyWeapon };
