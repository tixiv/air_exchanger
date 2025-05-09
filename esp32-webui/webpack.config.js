const HtmlWebpackPlugin = require('html-webpack-plugin');
const CopyPlugin = require('copy-webpack-plugin');
const path = require('path');

module.exports = {
    mode: 'production',
    cache: false,
    entry: {
        index: './src/index.ts',
        wifi_setup: './src/wifi_setup.ts',
    },
    output: {
        filename: '[name].bundle.js',
        path: path.resolve(__dirname, 'dist'),
        clean: true,
    },
    module: {
        rules: [
            {
                test: /\.ts$/,
                use: 'ts-loader',
                exclude: /node_modules/
            },
            {
                test: /\.css$/,
                use: ['style-loader', 'css-loader'],
            }
        ]
    },
    resolve: {
        extensions: ['.ts', '.js']
    },
    plugins: [
        new HtmlWebpackPlugin({
            template: 'src/index.html',
            filename: 'index.html',
            chunks: ['index'],
        }),
        new HtmlWebpackPlugin({
            template: 'src/wifi_setup.html',
            filename: 'wifi_setup.html',
            chunks: ['wifi_setup'],
        }),
        new CopyPlugin({
            patterns: [
                { from: 'src/nav.html', to: 'nav.html' }
            ]
        })
    ]
};
