const path = require('path')
const CopyPlugin = require('copy-webpack-plugin')

module.exports = {
  mode: 'development',
  devtool: 'source-map',
  entry: [
    './src/index.js',
    './src/routing.js',
    './src/map.js',
    './src/comm.js',
    './src/schedule.js',
    './src/utils/utils.js'
  ],
  output: {
    path: path.resolve(__dirname, 'dist/'),
    filename: 'bundle.js',
    publicPath: '/'
  },
  performance: {
    hints: false
  },
  plugins: [
    new CopyPlugin({
      patterns: [
        {
          from: 'style/index.css',
          to: 'styles/index.css',
          force: true
        },
        // fontawesome
        {
          from: 'node_modules/@fortawesome/fontawesome-free/css/all.min.css',
          to: 'styles/fa/css/all.css',
          force: true
        },
        {
          from: 'node_modules/@fortawesome/fontawesome-free/webfonts/',
          to: 'styles/fa/webfonts/',
          force: true
        },
        // leaflet
        {
          from: 'node_modules/leaflet/dist/leaflet.css',
          to: 'styles/leaflet.css',
          force: true
        },
        // bootstrap
        {
          from: 'node_modules/bootstrap/dist/css/bootstrap.min.css',
          to: 'styles/bootstrap.min.css',
          force: true
        }
      ]
    })
  ]
}
