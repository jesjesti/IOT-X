import react from '@vitejs/plugin-react'
import { isIPv4 } from 'node:net'
import { defineConfig } from 'vite'

const deviceRoutes = new Set([
  '/blink/fast',
  '/blink/slow',
  '/blink/stop',
  '/status',
])

function isPrivateIPv4(address: string) {
  if (!isIPv4(address)) {
    return false
  }

  const [first, second] = address.split('.').map(Number)
  return (
    first === 10 ||
    (first === 172 && second >= 16 && second <= 31) ||
    (first === 192 && second === 168) ||
    (first === 169 && second === 254)
  )
}

function isLoopbackAddress(address: string | undefined) {
  return (
    address === '::1' ||
    address === '127.0.0.1' ||
    address?.startsWith('::ffff:127.') === true
  )
}

export default defineConfig({
  plugins: [
    react(),
    {
      name: 'arduino-dev-proxy',
      configureServer(server) {
        server.middlewares.use('/arduino-proxy', (request, response) => {
          if (!isLoopbackAddress(request.socket.remoteAddress)) {
            response.statusCode = 403
            response.end('This development proxy is only available from localhost.')
            return
          }

          const route = request.url?.split('?')[0] ?? ''
          const addressHeader = request.headers['x-arduino-address']
          const address = Array.isArray(addressHeader)
            ? addressHeader[0]
            : addressHeader

          if (request.method !== 'GET' || !deviceRoutes.has(route)) {
            response.statusCode = 404
            response.end('Not found')
            return
          }

          if (!address || !isPrivateIPv4(address)) {
            response.statusCode = 400
            response.setHeader('Content-Type', 'application/json')
            response.end(JSON.stringify({ error: 'Enter a private IPv4 address.' }))
            return
          }

          void fetch(`http://${address}${route}`, {
            signal: AbortSignal.timeout(5000),
          })
            .then(async (deviceResponse) => {
              response.statusCode = deviceResponse.status
              response.setHeader(
                'Content-Type',
                deviceResponse.headers.get('content-type') ?? 'application/json',
              )
              response.end(await deviceResponse.text())
            })
            .catch(() => {
              response.statusCode = 502
              response.setHeader('Content-Type', 'application/json')
              response.end(
                JSON.stringify({
                  error: `Could not reach the Arduino at ${address}. Check its IP and that this computer is on the same network.`,
                }),
              )
            })
        })
      },
    },
  ],
})
