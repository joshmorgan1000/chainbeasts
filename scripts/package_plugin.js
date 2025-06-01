#!/usr/bin/env node
import { promises as fs } from 'fs';
import { join, dirname } from 'path';
import { execSync } from 'child_process';
import { fileURLToPath } from 'url';

async function main() {
  if (process.argv.length < 3) {
    console.error('Usage: package_plugin.js <plugin_dir>');
    process.exit(1);
  }
  const dir = process.argv[2];
  const manifestPath = join(dir, 'plugin.json');
  let manifest;
  try {
    manifest = JSON.parse(await fs.readFile(manifestPath, 'utf8'));
  } catch (err) {
    console.error(`Failed to read ${manifestPath}: ${err.message}`);
    process.exit(1);
  }
  const { name, version, harmonics_version } = manifest;
  if (!name || !version) {
    console.error('plugin.json must contain "name" and "version" fields');
    process.exit(1);
  }
  const __dirname = dirname(fileURLToPath(import.meta.url));
  const versionFile = join(
    __dirname,
    '..',
    'third_party',
    'harmonics',
    'include',
    'harmonics',
    'version.hpp'
  );
  let current = 0;
  try {
    const data = await fs.readFile(versionFile, 'utf8');
    const match = data.match(/return\s+(\d+)/);
    if (match) current = Number(match[1]);
  } catch {
    // ignore errors
  }
  if (harmonics_version && Number(harmonics_version) !== current) {
    console.error(
      `Harmonics version mismatch: plugin requires ${harmonics_version}, current ${current}`
    );
    process.exit(1);
  }
  const archive = `${name}-${version}.tar.gz`;
  execSync(`tar -czf ${archive} -C ${dir} .`, { stdio: 'inherit' });
  console.log(`Created ${archive}`);
}

main().catch((err) => {
  console.error(err);
  process.exit(1);
});
