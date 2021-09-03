module.exports = {
  env: {
    browser: true,
    es2021: true
  },
  extends: [
    'standard'
  ],
  parserOptions: {
    ecmaVersion: 12,
    sourceType: 'module'
  },
  rules: {
    indent: 'off'
  },
  ignorePatterns: [
    '*.html',
    '*.css',
    'package*.json',
    'dist/',
    'node_modules/'
  ]
}
