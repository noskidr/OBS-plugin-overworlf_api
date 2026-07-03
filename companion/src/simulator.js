'use strict';

/*
 * GamePulse Companion — Valorant match simulator.
 *
 * Emits a plausible stream of GEP-shaped Valorant events so the whole pipeline
 * (companion → WS → OBS plugin → chapters/clips/overlay/exports) can be tested
 * without launching Valorant. Enable with `--gp-simulate` or from the tray.
 *
 * It feeds the SAME normalizer path as real GEP by emitting raw {key,value}
 * events, so what you test is what ships.
 */

const EventEmitter = require('events');

class Simulator extends EventEmitter {
  constructor() {
    super();
    this.timer = null;
    this.round = 0;
    this.step = 0;
  }

  get running() {
    return this.timer !== null;
  }

  start() {
    if (this.timer) return;
    this.round = 0;
    this.step = 0;
    this.emit('game', 'detected', { id: 21640, name: 'VALORANT' });
    this.emit('gep-event', { gameId: 21640, feature: 'match_info', key: 'match_start', value: '' });
    this.emit('log', 'info', 'simulator started — feeding a mock Valorant match');
    this.timer = setInterval(() => this._tick(), 2200);
  }

  stop() {
    if (!this.timer) return;
    clearInterval(this.timer);
    this.timer = null;
    this.emit('gep-event', { gameId: 21640, feature: 'match_info', key: 'match_end', value: '' });
    this.emit('game', 'closed', { id: 21640, name: 'VALORANT' });
    this.emit('log', 'info', 'simulator stopped');
  }

  _tick() {
    // A scripted round: shopping → combat → some kills → an ace → spike → end.
    const script = [
      () => this._info('round_phase', 'shopping'),
      () => this._info('round_number', String(++this.round)),
      () => this._info('round_phase', 'combat'),
      () => this._killFeed('Vandal', 'PhoenixDown', true),
      () => this._event('kill', 1),
      () => this._killFeed('Sheriff', 'Sova_Main', true),
      () => this._event('kill', 2),
      () => this._killFeed('Operator', 'JettAndy', false),
      () => this._event('kill', 3),
      () => this._killFeed('Vandal', 'SageWall', true),
      () => this._event('kill', 4),
      () => this._killFeed('Vandal', 'OmenSmoke', true),
      () => this._event('kill', 5), // 5th -> plugin derives ACE
      () => this._event('spike_defused', ''),
      () => this._info('round_phase', 'end'),
    ];

    const fn = script[this.step % script.length];
    fn();
    this.step++;

    // occasional death to break multikill chains between rounds
    if (this.step % script.length === 0) {
      this._event('death', 1);
    }
  }

  _event(key, value) {
    this.emit('gep-event', { gameId: 21640, feature: key === 'death' ? 'death' : 'match_info', key, value });
  }

  _info(key, value) {
    this.emit('gep-info', { gameId: 21640, feature: 'match_info', category: 'match_info', key, value });
  }

  _killFeed(weapon, victim, headshot) {
    const value = JSON.stringify({
      attacker: 'You',
      victim,
      is_attacker_teammate: true,
      is_victim_teammate: false,
      weapon: 'TX_Hud_AR_' + (weapon === 'Vandal' ? 'Vandal' : weapon === 'Operator' ? 'Sniper' : 'Standard'),
      headshot: !!headshot,
    });
    this.emit('gep-event', { gameId: 21640, feature: 'match_info', key: 'kill_feed', value });
  }
}

module.exports = { Simulator };
