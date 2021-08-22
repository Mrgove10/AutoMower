const path = require('path');

module.exports = {
  entry: [
    './src/index.js',
    './src/routing.js',
    './src/map.js',
    './src/comm.js',
  ],
  output: {
    path: path.resolve(__dirname, 'dist'),
    filename: 'bundle.js',
  },
  mode: 'development',
  devServer: {
    static: {
      directory: path.join(__dirname, ''),
    },
    compress: true,
    port: 8080,
  },
};