# Security Policy

## Supported Versions

Bash++ is still in beta. Before 1.0, the only supported version is the latest version.

## Reporting a Vulnerability

Please **do not** open public issues for security-sensitive reports.

Instead, please send an [encrypted](https://rail5.org/pgpkey.txt) email to dev@bpp.sh with the following:

 - A description of the impact (what an attacker can do)
 - A description of the vulnerability (what the issue is)
 - Version(s) affected (if known)
 - A minimal proof-of-concept input that demonstrates the vulnerability (if possible)
 - Any crash output, logs, or other debugging information (if applicable/available)
 - *(Optional, would be appreciated)* `git blame` information on the relevant code

If the report involves the language server or an editor extension, please also include the name and version of the editor and extension.

## Coordinated Disclosure

Reports will be acknowledged as soon as possible, but please bear in mind that this is a volunteer project. Please allow a reasonable time for a patch before publishing details. If you have a preferred timeline for disclosure, please include that in your report.

## Security Updates

Security updates will be released as soon as a patch is available. If the vulnerability is severe, a patch may be released before a full disclosure writeup is available.

Release notes will reference the issue at a high level; please mention in your email whether you prefer to be credited by name or anonymously.

Writeups for significant vulnerabilities may be published on [log.bpp.sh](https://log.bpp.sh).
