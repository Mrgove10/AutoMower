const path = require('path');

module.exports = {
  mode: 'development',
  devtool: 'source-map',
  entry: [
    './src/index.js',
    './src/routing.js',
    './src/map.js',
    './src/comm.js'
  ],
  output: {
    path: path.resolve(__dirname, 'dist/'),
    filename: 'bundle.js',
    publicPath: '/'
  },
  module: {
    rules: [
      {
        test: /\.css$/i,
        use: ["style-loader", "css-loader"],
      },
    ],
  },
};