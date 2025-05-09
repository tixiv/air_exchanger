const path = require('path');
const fs = require('fs-extra');
const { glob } = require('glob');
const zlib = require('zlib');
const { createReadStream, createWriteStream } = require('fs');

const distDir = path.resolve(__dirname, 'dist');
const spiffsDir = path.resolve(__dirname, 'spiffs');

console.log('Cleaning SPIFFS directory...');
fs.emptyDirSync(spiffsDir);

console.log('Scanning dist/ for files...');
glob(`${distDir}/**/*.*`).then(async (files) => {
  console.log(`Found ${files.length} files in dist/`);

  if (files.length === 0) {
    console.warn('No files found. Did the build run? Is the dist path correct?');
    return;
  }

  for (const file of files) {
    const relPath = path.relative(distDir, file);
    const destPath = path.join(spiffsDir, relPath);

    try {
      await fs.ensureDir(path.dirname(destPath));
      await fs.copy(file, destPath);
      console.log(`Copied: ${relPath}`);

      const gzDest = destPath + '.gz';
      await gzipFile(file, gzDest);
      console.log(`Gzipped: ${relPath}.gz`);
    } catch (err) {
      console.error(`Error processing file ${relPath}:`, err);
    }
  }

  console.log('SPIFFS preparation complete.');
}).catch((err) => {
  console.error('Error during glob:', err);
});

function gzipFile(inputPath, outputPath) {
  return new Promise((resolve, reject) => {
    const gzip = zlib.createGzip({ level: 9 });
    const source = createReadStream(inputPath);
    const destination = createWriteStream(outputPath);

    source.pipe(gzip).pipe(destination)
      .on('finish', resolve)
      .on('error', reject);
  });
}
