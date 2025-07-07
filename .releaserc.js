// .releaserc.js

console.log('--- .releaserc.js: STARTING CONFIGURATION ---');

// --- Base Configuration ---
const isPrerelease = process.env.IS_PRERELEASE === 'true';
const branchName = process.env.GITHUB_REF_NAME || 'main';
const prereleaseChannel = process.env.PRERELEASE_CHANNEL || 'rc';
const forceReleaseType = process.env.FORCE_RELEASE_TYPE;

console.log(`- Detected Branch Name: [${branchName}]`);
console.log(`- Is Prerelease?: [${isPrerelease}]`);
console.log(`- Prerelease Channel: [${prereleaseChannel}]`);

const config = {
  branches: ['main'],
  plugins: [
    // commit-analyzer will be replaced below if FORCE_RELEASE_TYPE is defined
    '@semantic-release/commit-analyzer',
    '@semantic-release/release-notes-generator',
    ['@semantic-release/changelog', { changelogFile: 'CHANGELOG.md' }],
    [
      '@semantic-release/github',
      {
        prerelease: isPrerelease,
      }
    ],
    [
      '@semantic-release/git',
      {
        assets: ['CHANGELOG.md'],
        message: 'chore(release): ${nextRelease.version} [skip ci]\n\n${nextRelease.notes}',
      }
    ],
  ],
};

// --- Dynamic Branch Logic ---
if (isPrerelease) {
  if (branchName && prereleaseChannel) {
    console.log(`>>> Configuring for PRE-RELEASE. Adding branch "${branchName}" to the configuration.`);
    config.branches.push({
      name: branchName,
      prerelease: prereleaseChannel,
    });
  } else {
    console.error('!!! ERROR: isPrerelease is true, but branchName or prereleaseChannel is missing!');
  }
} else {
  console.log('>>> Configuring for a REGULAR release.');
}

// --- Dynamic Force Release Type Logic ---
if (forceReleaseType) {
  console.log(`>>> Forcing a release of type: [${forceReleaseType}]`);
  config.plugins[0] = [
    '@semantic-release/commit-analyzer',
    {
      releaseRules: [
        { release: forceReleaseType }
      ]
    }
  ];
}

console.log('--- .releaserc.js: FINAL CONFIGURATION ---');
console.log(JSON.stringify(config, null, 2));
console.log('------------------------------------------');

module.exports = config;