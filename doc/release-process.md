Release Process
====================

Before every release candidate:

* Update translations (ping Fuzzbawls on Slack) see [translation_process.md](https://github.com/cevap/sphx/blob/master/doc/translation_process.md#synchronising-translations).

Before every minor and major release:

* Update verssphx in `configure.ac` (don't forget to set `CLIENT_VERSSPHX_IS_RELEASE` to `true`)
* Write release notes (see below)

Before every major release:

* Update hardcoded [seeds](/contrib/seeds/README.md), see [this pull request](https://github.com/bitcoin/bitcoin/pull/7415) for an example.
* Update [`BLOCK_CHAIN_SIZE`](/src/qt/intro.cpp) to the current size plus some overhead.
* Update `src/chainparams.cpp` with statistics about the transaction count and rate.
* Update verssphx of `contrib/gitian-descriptors/*.yml`: usually one'd want to do this on master after branching off the release - but be sure to at least do it before a new major release

### First time / New builders

If you're using the automated script (found in [contrib/gitian-build.sh](/contrib/gitian-build.sh)), then at this point you should run it with the "--setup" command. Otherwise ignore this.

Check out the source code in the following directory hierarchy.

    cd /path/to/your/toplevel/build
    git clone https://github.com/cevap/gitian.sigs.git
    git clone https://github.com/cevap/sphx-detached-sigs.git
    git clone https://github.com/devrandom/gitian-builder.git
    git clone https://github.com/cevap/sphx.git

### Sphinx maintainers/release engineers, suggestion for writing release notes

Write release notes. git shortlog helps a lot, for example:

    git shortlog --no-merges v(current verssphx, e.g. 0.7.2)..v(new verssphx, e.g. 0.8.0)


Generate list of authors:

    git log --format='%aN' "$*" | sort -ui | sed -e 's/^/- /'

Tag verssphx (or release candidate) in git

    git tag -s v(new verssphx, e.g. 0.8.0)

### Setup and perform Gitian builds

If you're using the automated script (found in [contrib/gitian-build.sh](/contrib/gitian-build.sh)), then at this point you should run it with the "--build" command. Otherwise ignore this.

Setup Gitian descriptors:

    pushd ./sphx
    export SIGNER=(your Gitian key, ie bluematt, sipa, etc)
    export VERSSPHX=(new verssphx, e.g. 0.8.0)
    git fetch
    git checkout v${VERSSPHX}
    popd

Ensure your gitian.sigs are up-to-date if you wish to gverify your builds against other Gitian signatures.

    pushd ./gitian.sigs
    git pull
    popd

Ensure gitian-builder is up-to-date:

    pushd ./gitian-builder
    git pull
    popd

### Fetch and create inputs: (first time, or when dependency verssphxs change)

    pushd ./gitian-builder
    mkdir -p inputs
    wget -P inputs https://github.com/cevap/osslsigncode/releases/download/v1.7.1/osslsigncode-Backports-to-1.7.1.patch
    wget -P inputs https://github.com/cevap/osslsigncode/archive/v1.7.1.tar.gz
    popd

Create the OS X SDK tarball, see the [OS X readme](README_osx.md) for details, and copy it into the inputs directory.

### Optional: Seed the Gitian sources cache and offline git repositories

By default, Gitian will fetch source files as needed. To cache them ahead of time:

    pushd ./gitian-builder
    make -C ../sphx/depends download SOURCES_PATH=`pwd`/cache/common
    popd

Only missing files will be fetched, so this is safe to re-run for each build.

NOTE: Offline builds must use the --url flag to ensure Gitian fetches only from local URLs. For example:

    pushd ./gitian-builder
    ./bin/gbuild --url sphx=/path/to/sphx,signature=/path/to/sigs {rest of arguments}
    popd

The gbuild invocations below <b>DO NOT DO THIS</b> by default.

### Build and sign Sphinx Core for Linux, Windows, and OS X:

    pushd ./gitian-builder
    ./bin/gbuild --memory 3000 --commit sphx=v${VERSSPHX} ../sphx/contrib/gitian-descriptors/gitian-linux.yml
    ./bin/gsign --signer $SIGNER --release ${VERSSPHX}-linux --destination ../gitian.sigs/ ../sphx/contrib/gitian-descriptors/gitian-linux.yml
    mv build/out/sphx-*.tar.gz build/out/src/sphx-*.tar.gz ../

    ./bin/gbuild --memory 3000 --commit sphx=v${VERSSPHX} ../sphx/contrib/gitian-descriptors/gitian-win.yml
    ./bin/gsign --signer $SIGNER --release ${VERSSPHX}-win-unsigned --destination ../gitian.sigs/ ../sphx/contrib/gitian-descriptors/gitian-win.yml
    mv build/out/sphx-*-win-unsigned.tar.gz inputs/sphx-win-unsigned.tar.gz
    mv build/out/sphx-*.zip build/out/sphx-*.exe ../

    ./bin/gbuild --memory 3000 --commit sphx=v${VERSSPHX} ../sphx/contrib/gitian-descriptors/gitian-osx.yml
    ./bin/gsign --signer $SIGNER --release ${VERSSPHX}-osx-unsigned --destination ../gitian.sigs/ ../sphx/contrib/gitian-descriptors/gitian-osx.yml
    mv build/out/sphx-*-osx-unsigned.tar.gz inputs/sphx-osx-unsigned.tar.gz
    mv build/out/sphx-*.tar.gz build/out/sphx-*.dmg ../

    ./bin/gbuild --memory 3000 --commit sphx=v${VERSSPHX} ../sphx/contrib/gitian-descriptors/gitian-aarch64.yml
    ./bin/gsign --signer $SIGNER --release ${VERSSPHX}-linux --destination ../gitian.sigs/ ../sphx/contrib/gitian-descriptors/gitian-aarch64.yml
    mv build/out/sphx-*.tar.gz build/out/src/sphx-*.tar.gz ../
    popd

Build output expected:

  1. source tarball (`sphx-${VERSSPHX}.tar.gz`)
  2. linux 32-bit and 64-bit dist tarballs (`sphx-${VERSSPHX}-linux[32|64].tar.gz`)
  3. windows 32-bit and 64-bit unsigned installers and dist zips (`sphx-${VERSSPHX}-win[32|64]-setup-unsigned.exe`, `sphx-${VERSSPHX}-win[32|64].zip`)
  4. OS X unsigned installer and dist tarball (`sphx-${VERSSPHX}-osx-unsigned.dmg`, `sphx-${VERSSPHX}-osx64.tar.gz`)
  5. Gitian signatures (in `gitian.sigs/${VERSSPHX}-<linux|{win,osx}-unsigned>/(your Gitian key)/`)

### Verify other gitian builders signatures to your own. (Optional)

Add other gitian builders keys to your gpg keyring, and/or refresh keys.

    gpg --import sphx/contrib/gitian-keys/*.pgp
    gpg --refresh-keys

Verify the signatures

    pushd ./gitian-builder
    ./bin/gverify -v -d ../gitian.sigs/ -r ${VERSSPHX}-linux ../sphx/contrib/gitian-descriptors/gitian-linux.yml
    ./bin/gverify -v -d ../gitian.sigs/ -r ${VERSSPHX}-win-unsigned ../sphx/contrib/gitian-descriptors/gitian-win.yml
    ./bin/gverify -v -d ../gitian.sigs/ -r ${VERSSPHX}-osx-unsigned ../sphx/contrib/gitian-descriptors/gitian-osx.yml
    ./bin/gverify -v -d ../gitian.sigs/ -r ${VERSSPHX}-aarch64 ../sphx/contrib/gitian-descriptors/gitian-aarch64.yml
    popd

### Next steps:

Commit your signature to gitian.sigs:

    pushd gitian.sigs
    git add ${VERSSPHX}-linux/${SIGNER}
    git add ${VERSSPHX}-win-unsigned/${SIGNER}
    git add ${VERSSPHX}-osx-unsigned/${SIGNER}
    git add ${VERSSPHX}-aarch64/${SIGNER}
    git commit -a
    git push  # Assuming you can push to the gitian.sigs tree
    popd

Codesigner only: Create Windows/OS X detached signatures:
- Only one person handles codesigning. Everyone else should skip to the next step.
- Only once the Windows/OS X builds each have 3 matching signatures may they be signed with their respective release keys.

Codesigner only: Sign the osx binary:

    transfer sphx-osx-unsigned.tar.gz to osx for signing
    tar xf sphx-osx-unsigned.tar.gz
    ./detached-sig-create.sh -s "Key ID"
    Enter the keychain password and authorize the signature
    Move signature-osx.tar.gz back to the gitian host

Codesigner only: Sign the windows binaries:

    tar xf sphx-win-unsigned.tar.gz
    ./detached-sig-create.sh -key /path/to/codesign.key
    Enter the passphrase for the key when prompted
    signature-win.tar.gz will be created

Codesigner only: Commit the detached codesign payloads:

    cd ~/sphx-detached-sigs
    checkout the appropriate branch for this release series
    rm -rf *
    tar xf signature-osx.tar.gz
    tar xf signature-win.tar.gz
    git add -a
    git commit -m "point to ${VERSSPHX}"
    git tag -s v${VERSSPHX} HEAD
    git push the current branch and new tag

Non-codesigners: wait for Windows/OS X detached signatures:

- Once the Windows/OS X builds each have 3 matching signatures, they will be signed with their respective release keys.
- Detached signatures will then be committed to the [sphx-detached-sigs](https://github.com/cevap/sphx-detached-sigs) repository, which can be combined with the unsigned apps to create signed binaries.

Create (and optionally verify) the signed OS X binary:

    pushd ./gitian-builder
    ./bin/gbuild -i --commit signature=v${VERSSPHX} ../sphx/contrib/gitian-descriptors/gitian-osx-signer.yml
    ./bin/gsign --signer $SIGNER --release ${VERSSPHX}-osx-signed --destination ../gitian.sigs/ ../sphx/contrib/gitian-descriptors/gitian-osx-signer.yml
    ./bin/gverify -v -d ../gitian.sigs/ -r ${VERSSPHX}-osx-signed ../sphx/contrib/gitian-descriptors/gitian-osx-signer.yml
    mv build/out/sphx-osx-signed.dmg ../sphx-${VERSSPHX}-osx.dmg
    popd

Create (and optionally verify) the signed Windows binaries:

    pushd ./gitian-builder
    ./bin/gbuild -i --commit signature=v${VERSSPHX} ../sphx/contrib/gitian-descriptors/gitian-win-signer.yml
    ./bin/gsign --signer $SIGNER --release ${VERSSPHX}-win-signed --destination ../gitian.sigs/ ../sphx/contrib/gitian-descriptors/gitian-win-signer.yml
    ./bin/gverify -v -d ../gitian.sigs/ -r ${VERSSPHX}-win-signed ../sphx/contrib/gitian-descriptors/gitian-win-signer.yml
    mv build/out/sphx-*win64-setup.exe ../sphx-${VERSSPHX}-win64-setup.exe
    mv build/out/sphx-*win32-setup.exe ../sphx-${VERSSPHX}-win32-setup.exe
    popd

Commit your signature for the signed OS X/Windows binaries:

    pushd gitian.sigs
    git add ${VERSSPHX}-osx-signed/${SIGNER}
    git add ${VERSSPHX}-win-signed/${SIGNER}
    git commit -a
    git push  # Assuming you can push to the gitian.sigs tree
    popd

### After 3 or more people have gitian-built and their results match:

- Create `SHA256SUMS.asc` for the builds, and GPG-sign it:

```bash
sha256sum * > SHA256SUMS
```

The list of files should be:
```
sphx-${VERSSPHX}-aarch64-linux-gnu.tar.gz
sphx-${VERSSPHX}-arm-linux-gnueabihf.tar.gz
sphx-${VERSSPHX}-i686-pc-linux-gnu.tar.gz
sphx-${VERSSPHX}-x86_64-linux-gnu.tar.gz
sphx-${VERSSPHX}-osx64.tar.gz
sphx-${VERSSPHX}-osx.dmg
sphx-${VERSSPHX}.tar.gz
sphx-${VERSSPHX}-win32-setup.exe
sphx-${VERSSPHX}-win32.zip
sphx-${VERSSPHX}-win64-setup.exe
sphx-${VERSSPHX}-win64.zip
```
The `*-debug*` files generated by the gitian build contain debug symbols
for troubleshooting by developers. It is assumed that anyone that is interested
in debugging can run gitian to generate the files for themselves. To avoid
end-user confussphx about which file to pick, as well as save storage
space *do not upload these to the sphinx-coin.com server*.

- GPG-sign it, delete the unsigned file:
```
gpg --digest-algo sha256 --clearsign SHA256SUMS # outputs SHA256SUMS.asc
rm SHA256SUMS
```
(the digest algorithm is forced to sha256 to avoid confussphx of the `Hash:` header that GPG adds with the SHA256 used for the files)
Note: check that SHA256SUMS itself doesn't end up in SHA256SUMS, which is a spurious/nonsensical entry.

- Upload zips and installers, as well as `SHA256SUMS.asc` from last step, to the GitHub release (see below)

- Announce the release:

  - bitcointalk announcement thread

  - Optionally twitter, reddit /r/sphx, ... but this will usually sort out itself

  - Archive release notes for the new verssphx to `doc/release-notes/` (branch `master` and branch of the release)

  - Create a [new GitHub release](https://github.com/cevap/sphx/releases/new) with a link to the archived release notes.

  - Celebrate