import { useState } from 'react'
import axios from 'axios'
import './App.css'

type DeviceStatus = {
  blinking?: boolean
  interval?: number
  led?: string
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

function App() {
  const [deviceAddress, setDeviceAddress] = useState('192.168.1.11')
  const [deviceStatus, setDeviceStatus] = useState<DeviceStatus | null>(null)
  const [lastResponse, setLastResponse] = useState<{
    label: string
    data: unknown
  } | null>(null)
  const [errorMessage, setErrorMessage] = useState('')
  const [loadingAction, setLoadingAction] = useState<string | null>(null)

  async function sendRequest(action: Action) {
    let url: URL
    const address = deviceAddress.trim()
    const addressWithProtocol = /^https?:\/\//i.test(address)
      ? address
      : `http://${address}`

    try {
      url = new URL(addressWithProtocol)
    } catch {
      setErrorMessage('Enter a valid Arduino IP address or hostname.')
      return
    }

    if (
      !['http:', 'https:'].includes(url.protocol) ||
      url.pathname !== '/' ||
      url.search ||
      url.hash
    ) {
      setErrorMessage('Enter only the Arduino IP address or hostname.')
      return
    }

    setLoadingAction(action.label)
    setErrorMessage('')

    try {
      const response = await axios.get<DeviceStatus>(
        `/arduino-proxy${action.path}`,
        {
          headers: { 'x-arduino-address': url.hostname },
          timeout: 6000,
        },
      )
      setLastResponse({ label: action.label, data: response.data })

      if (action.path === '/status') {
        setDeviceStatus(response.data)
      }
    } catch (error: unknown) {
      setLastResponse(null)
      if (axios.isAxiosError(error) && error.response) {
        const proxyError = error.response.data as { error?: string } | undefined
        setErrorMessage(
          proxyError?.error ??
            `The Arduino returned HTTP ${error.response.status}. Check the device and try again.`,
        )
      } else {
        setErrorMessage(
          'Could not reach the local development proxy. Restart the Vite dev server and try again.',
        )
      }
    } finally {
      setLoadingAction(null)
    }
  }

  return (
    <main className="app-shell">
      <header className="page-header">
        <div className="brand-mark" aria-hidden="true">
          LED
        </div>
        <div>
          <p className="eyebrow">Arduino UNO R4 WiFi</p>
          <h1>Device control</h1>
          <p className="subtitle">Control the onboard LED and check its status.</p>
        </div>
      </header>

      <section className="panel connection-panel" aria-labelledby="connection-title">
        <div className="section-heading">
          <div>
            <p className="eyebrow">Connection</p>
            <h2 id="connection-title">Arduino address</h2>
          </div>
          <span className="connection-indicator">
            <span className={`indicator-dot${errorMessage ? ' is-error' : ''}`} />
            {errorMessage ? 'Connection issue' : 'Ready to connect'}
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
            placeholder="192.168.1.11"
            value={deviceAddress}
            onChange={(event) => setDeviceAddress(event.target.value)}
            onKeyDown={(event) => {
              if (event.key === 'Enter') {
                void sendRequest(actions[3])
              }
            }}
          />
        </div>
      </section>

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
          {errorMessage && (
            <p className="error-message" role="alert">
              {errorMessage}
            </p>
          )}
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
