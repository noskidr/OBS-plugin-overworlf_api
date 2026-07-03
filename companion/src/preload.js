'use strict';

const { contextBridge, ipcRenderer } = require('electron');

contextBridge.exposeInMainWorld('gp', {
  getSettings: () => ipcRenderer.invoke('get-settings'),
  saveSettings: (s) => ipcRenderer.invoke('save-settings', s),
  toggleSimulator: () => ipcRenderer.invoke('toggle-simulator'),
  reconnect: () => ipcRenderer.invoke('reconnect'),
  status: () => ipcRenderer.invoke('status'),
  onLog: (cb) => ipcRenderer.on('log', (_e, line) => cb(line)),
  onSettings: (cb) => ipcRenderer.on('settings', (_e, s) => cb(s)),
});
