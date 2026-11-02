import { useState } from 'react'
import axios from 'axios'
import './App.css'

type DeviceStatus = {
  blinking?: boolean
  interval?: number
  led?: string
}

type WifiStatus = {
  mode: 'provisioning' | 'station'
  connected: boolean
  ssid: string
  ip: string
}

type WifiNetwork = {
  ssid: string
  rssi: number
}

type Action = {
  label: string
  path: string
}

const actions: Action[] = [
  { label: 'Fast blink', path: '/blink/fast' },
  { label: 'Slow blink', path: '/blink/slow' },
  { label: 'Stop blinking', path: '/blink/stop' },
  { label: 'Refresh status', path: '/status' },
]

async function requestDevice<T>(
  deviceAddress: string,
  path: string,
  method: 'GET' | 'POST' | 'DELETE' = 'GET',
  data?: unknown,
): Promise<T> {
  const address = deviceAddress.trim()
  const addressWithProtocol = /^https?:\/\//i.test(address)
    ? address
    : `http://${address}`
  let url: URL

  try {
    url = new URL(addressWithProtocol)
  } catch {
    throw new Error('Enter a valid Arduino IP address or hostname.')
  }

  if (
    !['http:', 'https:'].includes(url.protocol) ||
    url.pathname !== '/' ||
    url.search ||
    url.hash ||
    url.port
  ) {
    throw new Error('Enter only the Arduino IP address or hostname.')
  }

  const response = await axios.request<T>({
    url: `/arduino-proxy${path}`,
    method,
    headers: { 'x-arduino-address': url.hostname },
    data,
    timeout: 20000,
  })
  return response.data
}

function errorMessage(error: unknown, fallback: string) {
  if (axios.isAxiosError(error)) {
    if (error.response) {
      const proxyError = error.response.data as { error?: string } | undefined
      return (
        proxyError?.error ??
        `The Arduino returned HTTP ${error.response.status}. Check the device and try again.`
      )
    }
    return 'Could not reach the local development proxy. Restart the Vite dev server and try again.'
  }
  return error instanceof Error ? error.message : fallback
}

function App() {
  const [deviceAddress, setDeviceAddress] = useState('192.168.4.1')
  const [deviceStatus, setDeviceStatus] = useState<DeviceStatus | null>(null)
  const [wifiStatus, setWifiStatus] = useState<WifiStatus | null>(null)
  const [wifiNetworks, setWifiNetworks] = useState<WifiNetwork[]>([])
  const [selectedSsid, setSelectedSsid] = useState('')
  const [manualSsid, setManualSsid] = useState('')
  const [wifiPassword, setWifiPassword] = useState('')
  const [wifiMessage, setWifiMessage] = useState('')
  const [lastResponse, setLastResponse] = useState<{
    label: string
    data: unknown
  } | null>(null)
  const [error, setError] = useState('')
  const [loadingAction, setLoadingAction] = useState<string | null>(null)

  async function sendRequest(action: Action) {
    setLoadingAction(action.label)
    setError('')
    try {
      const data = await requestDevice<DeviceStatus>(deviceAddress, action.path)
      setLastResponse({ label: action.label, data })
      if (action.path === '/status') {
        setDeviceStatus(data)
      }
    } catch (requestError: unknown) {
      setLastResponse(null)
      setError(errorMessage(requestError, 'Could not reach the Arduino.'))
    } finally {
      setLoadingAction(null)
    }
  }

  async function refreshWifi() {
    setLoadingAction('Wi-Fi status')
    setError('')
    setWifiMessage('')
    try {
      const status = await requestDevice<WifiStatus>(deviceAddress, '/wifi/status')
      setWifiStatus(status)
      if (status.mode === 'provisioning') {
        const networks = await requestDevice<WifiNetwork[]>(
          deviceAddress,
          '/wifi/networks',
        )
        setWifiNetworks(networks)
        if (!selectedSsid && networks.length > 0) {
          setSelectedSsid(networks[0].ssid)
        }
      } else {
        setWifiNetworks([])
      }
    } catch (requestError: unknown) {
      setError(errorMessage(requestError, 'Could not read the Wi-Fi status.'))
    } finally {
      setLoadingAction(null)
    }
  }

  async function saveWifiCredentials() {
    const ssid = manualSsid || selectedSsid
    const passwordLength = new TextEncoder().encode(wifiPassword).length
    if (!ssid.trim()) {
      setError('Select a Wi-Fi network or enter its name.')
      return
    }
    if (
      passwordLength > 63 ||
      (passwordLength > 0 && passwordLength < 8)
    ) {
      setError('Wi-Fi passwords must be empty or 8-63 bytes long.')
      return
    }

    setLoadingAction('Save Wi-Fi')
    setError('')
    setWifiMessage('')
    try {
      await requestDevice(deviceAddress, '/wifi/credentials', 'POST', {
        ssid,
        password: wifiPassword,
      })
      setWifiStatus({
        mode: 'station',
        connected: false,
        ssid,
        ip: '',
      })
      setWifiNetworks([])
      setWifiPassword('')
      setWifiMessage(
        'Credentials saved. The device is switching to your home Wi-Fi. Connect this computer to that network and enter the Arduino’s new IP address above.',
      )
    } catch (requestError: unknown) {
      setError(errorMessage(requestError, 'Could not save Wi-Fi credentials.'))
    } finally {
      setLoadingAction(null)
    }
  }

  async function resetWifiCredentials() {
    if (
      !window.confirm(
        'Erase the saved Wi-Fi credentials and restart setup mode?',
      )
    ) {
      return
    }

    setLoadingAction('Reset Wi-Fi')
    setError('')
    setWifiMessage('')
    try {
      await requestDevice(deviceAddress, '/wifi/credentials', 'DELETE')
      setWifiStatus({
        mode: 'provisioning',
        connected: false,
        ssid: 'IIOT-X-WiFi',
        ip: '192.168.4.1',
      })
      setWifiNetworks([])
      setWifiMessage(
        'Credentials erased. Connect this computer to IIOT-X-WiFi, then scan for your home network.',
      )
    } catch (requestError: unknown) {
      setError(errorMessage(requestError, 'Could not reset Wi-Fi credentials.'))
    } finally {
      setLoadingAction(null)
    }
  }

  return (
    <main className="app-shell">
      <header className="page-header">
        <div className="brand-mark" aria-hidden="true">
          IOT-X
        </div>
        <div>
          <p className="eyebrow">Arduino UNO R4 WiFi</p>
          <h1>Device setup</h1>
          <p className="subtitle">
            Provision Wi-Fi and control the onboard LED.
          </p>
        </div>
      </header>

      <section className="panel connection-panel" aria-labelledby="connection-title">
        <div className="section-heading">
          <div>
            <p className="eyebrow">Connection</p>
            <h2 id="connection-title">Arduino address</h2>
          </div>
          <span className="connection-indicator">
            <span className={`indicator-dot${error ? ' is-error' : ''}`} />
            {error ? 'Connection issue' : 'Ready to connect'}
          </span>
        </div>
        <label htmlFor="device-address">Device private IPv4 address</label>
        <div className="address-field">
          <span className="address-prefix" aria-hidden="true">
            http://
          </span>
          <input
            id="device-address"
            type="text"
            inputMode="url"
            autoComplete="url"
            placeholder="192.168.4.1"
            value={deviceAddress}
            onChange={(event) => setDeviceAddress(event.target.value)}
            onKeyDown={(event) => {
              if (event.key === 'Enter') {
                void sendRequest(actions[3])
              }
            }}
          />
        </div>
        <p className="field-hint">
          Use 192.168.4.1 while connected to the IIOT-X-WiFi setup network.
        </p>
      </section>

      <section className="panel wifi-panel" aria-labelledby="wifi-title">
        <div className="section-heading">
          <div>
            <p className="eyebrow">Network configuration</p>
            <h2 id="wifi-title">Wi-Fi provisioning</h2>
          </div>
          {wifiStatus && (
            <span className={`wifi-badge${wifiStatus.connected ? ' is-connected' : ''}`}>
              {wifiStatus.mode === 'provisioning'
                ? 'Setup mode'
                : wifiStatus.connected
                  ? 'Connected'
                  : 'Connecting'}
            </span>
          )}
        </div>

        <p className="wifi-security-note">
          The IIOT-X-WiFi setup network is open, and credentials travel over
          unencrypted local HTTP. Provision only in a trusted environment.
        </p>

        {wifiStatus && (
          <p className="wifi-current">
            {wifiStatus.mode === 'provisioning'
              ? `Setup network: ${wifiStatus.ssid} (${wifiStatus.ip})`
              : `Home network: ${wifiStatus.ssid}${wifiStatus.ip ? ` · ${wifiStatus.ip}` : ''}`}
          </p>
        )}

        <div className="wifi-actions">
          <button
            className="action-button status-button"
            type="button"
            disabled={loadingAction !== null}
            onClick={() => void refreshWifi()}
          >
            {loadingAction === 'Wi-Fi status' ? 'Scanning…' : 'Scan networks'}
            {loadingAction === 'Wi-Fi status' && (
              <span className="button-spinner" aria-label="Loading" />
            )}
          </button>
          <button
            className="action-button reset-wifi-button"
            type="button"
            disabled={loadingAction !== null}
            onClick={() => void resetWifiCredentials()}
          >
            {loadingAction === 'Reset Wi-Fi' ? 'Resetting…' : 'Reset Wi-Fi credentials'}
            {loadingAction === 'Reset Wi-Fi' && (
              <span className="button-spinner" aria-label="Loading" />
            )}
          </button>
        </div>

        {(!wifiStatus || wifiStatus.mode === 'provisioning') && (
          <>
            <label htmlFor="home-network">Home Wi-Fi network</label>
            <select
              id="home-network"
              value={selectedSsid}
              onChange={(event) => {
                setSelectedSsid(event.target.value)
                setManualSsid('')
              }}
            >
              <option value="">
                {wifiNetworks.length ? 'Select a scanned network' : 'Scan to find networks'}
              </option>
              {wifiNetworks.map((network, index) => (
                <option key={`${network.ssid}-${index}`} value={network.ssid}>
                  {network.ssid || '(hidden network)'} · {network.rssi} dBm
                </option>
              ))}
            </select>

            <label htmlFor="manual-ssid">Or enter a hidden network name</label>
            <input
              id="manual-ssid"
              className="text-field"
              type="text"
              maxLength={32}
              autoComplete="off"
              value={manualSsid}
              onChange={(event) => setManualSsid(event.target.value)}
            />

            <label htmlFor="wifi-password">Wi-Fi password</label>
            <input
              id="wifi-password"
              className="text-field"
              type="password"
              autoComplete="new-password"
              value={wifiPassword}
              onChange={(event) => setWifiPassword(event.target.value)}
            />
            <p className="field-hint">
              Leave the password empty for an open home network. Password is not
              saved in this app.
            </p>
            <button
              className="action-button provision-button"
              type="button"
              disabled={loadingAction !== null}
              onClick={() => void saveWifiCredentials()}
            >
              {loadingAction === 'Save Wi-Fi' ? 'Saving…' : 'Save and connect'}
              {loadingAction === 'Save Wi-Fi' && (
                <span className="button-spinner" aria-label="Loading" />
              )}
            </button>
          </>
        )}

        {wifiMessage && (
          <p className="success-message" role="status">
            {wifiMessage}
          </p>
        )}
      </section>

      {error && (
        <p className="error-message global-error" role="alert">
          {error}
        </p>
      )}

      <div className="content-grid">
        <section className="panel controls-panel" aria-labelledby="controls-title">
          <div className="section-heading">
            <div>
              <p className="eyebrow">Remote controls</p>
              <h2 id="controls-title">LED commands</h2>
            </div>
          </div>
          <div className="button-grid">
            {actions.map((action) => (
              <button
                className={`action-button${action.path === '/blink/stop' ? ' stop-button' : ''}${action.path === '/status' ? ' status-button' : ''}`}
                key={action.path}
                type="button"
                disabled={loadingAction !== null}
                onClick={() => void sendRequest(action)}
              >
                <span>{action.label}</span>
                {loadingAction === action.label && (
                  <span className="button-spinner" aria-label="Loading" />
                )}
              </button>
            ))}
          </div>
        </section>

        <section className="panel status-panel" aria-labelledby="status-title">
          <div className="section-heading">
            <div>
              <p className="eyebrow">Live device response</p>
              <h2 id="status-title">Current status</h2>
            </div>
            {deviceStatus?.led && (
              <span
                className={`led-badge${deviceStatus.led.toUpperCase() === 'ON' ? ' led-on' : ''}`}
              >
                <span className="led-dot" />
                LED {deviceStatus.led}
              </span>
            )}
          </div>
          {deviceStatus ? (
            <dl className="status-details">
              <div>
                <dt>Blinking</dt>
                <dd>{deviceStatus.blinking ? 'On' : 'Off'}</dd>
              </div>
              <div>
                <dt>Interval</dt>
                <dd>
                  {typeof deviceStatus.interval === 'number'
                    ? `${deviceStatus.interval} ms`
                    : '—'}
                </dd>
              </div>
            </dl>
          ) : (
            <p className="empty-status">
              Select <strong>Refresh status</strong> to read the current device state.
            </p>
          )}
          {lastResponse && (
            <div className="response-block" aria-live="polite">
              <div className="response-heading">
                <span>{lastResponse.label} response</span>
                {loadingAction === null && <span className="response-success">Received</span>}
              </div>
              <pre>{JSON.stringify(lastResponse.data, null, 2)}</pre>
            </div>
          )}
        </section>
      </div>
      <footer className="page-footer">
        Requests are routed through the local Vite development proxy.
      </footer>
    </main>
  )
}

export default App
