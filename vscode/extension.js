const vscode = require('vscode');
const { LanguageClient, TransportKind } = require('vscode-languageclient');
const { execFile } = require('child_process');
const { promisify } = require('util');
const execFileAsync = promisify(execFile);

/** @type {LanguageClient | null} */
let client = null;

/**
 * `bpp-lsp --version` outputs:
 * Bash++ Language Server x.y.z
 * .. more lines
 * 
 * So here, we just call `--version`,
 * and scan the first line for [0-9]+\.[0-9]+\.[0-9]+
 * 
 * We return it as an array of 3 elements: [major, minor, patch]
 * If the version cannot be determined, we return [0, 0, 0]
 */
async function getServerVersion(serverPath) {
	try {
		const { stdout } = await execFileAsync(serverPath, ['--version']);
		const firstLine = stdout.split('\n')[0];
		const versionMatch = firstLine.match(/(\d+)\.(\d+)\.(\d+)/);
		if (versionMatch) {
			return versionMatch.slice(1, 4).map(num => parseInt(num, 10));
		}
	} catch (err) {
		vscode.window.showInformationMessage('Warning: Unable to determine Bash++ Language Server version. Some features may not work as expected. Please ensure the server is installed and accessible.');
	}
	return [0, 0, 0];
}

/**
 * Function to compare two version arrays [major, minor, patch]
 * Returns true if versionA >= versionB
 */
function isVersionGreaterOrEqual(versionA, versionB) {
	for (let i = 0; i < 3; i++) {
		if (versionA[i] > versionB[i]) {
			return true;
		} else if (versionA[i] < versionB[i]) {
			return false;
		}
	}
	return true; // versions are equal
}

async function stopClient() {
	if (client) {
		await client.stop();
		client = null;
	}
}

async function startClient(context) {
	if (client) {
		return; // already running
	}

	const config = vscode.workspace.getConfiguration('bashpp.languageServer');

	// Only start the language server if it is enabled in the configuration
	if (config.get('enabled', true)) {
		// If no path is specified, use the default 'bpp-lsp'
		const serverPath = config.get('path', 'bpp-lsp');

		// Do not continue with activation if the path is not found on the system
		try {
			const { execSync } = require('child_process');
			execSync(`which "${serverPath}"`, { stdio: 'ignore' });
		// If the command fails, it means the server path is not valid
		} catch (err) {
			return;
		}

		// First: get the server version to determine which features are supported
		const serverVersion = await getServerVersion(serverPath);
		console.log(`Bash++ Language Server version: ${serverVersion.join('.')}`);

		const args = ['--stdio'];

		if (!config.get('showWarnings', true)) {
			// If "showWarnings" is false, add the '-s' argument to suppress warnings
			args.push('-s');
		}

		if (config.get('log', false)) {
			// If logging is enabled, add the '--log' argument with the specified log file
			args.push('--log', config.get('logFile', '/tmp/bpp-lsp.log'));
		}

		// Add -I for each include path
		const includePaths = config.get('includePaths') || [];
		includePaths.forEach(path => {
			args.push(`-I${path}`);
		});

		// Get the target bash version
		// Note: '-b' was introduced in 0.8.0
		if (isVersionGreaterOrEqual(serverVersion, [0, 8, 0])) {
			const targetBashVersion = config.get('targetBashVersion', '5.2');
			args.push(`-b${targetBashVersion}`);
		} else {
			vscode.window.showInformationMessage('Bash++ Language Server version does not support target bash version configuration. Please upgrade to bpp-lsp 0.8.0 or later to use this feature.');
		}

		// Get the thread count for the language server
		// Note: '-j' was introduced in 0.8.11
		if (isVersionGreaterOrEqual(serverVersion, [0, 8, 11])) {
			const threadCount = config.get('threadCount', 0);
			if (threadCount > 0) {
				args.push(`-j${threadCount}`);
			}
		} else {
			vscode.window.showInformationMessage('Bash++ Language Server version does not support thread count configuration. Please upgrade to bpp-lsp 0.8.11 or later to use this feature.');
		}

		const serverOptions = {
			run: {
				command: serverPath,
				args,
				transport: TransportKind.stdio
			},

		};

		const clientOptions = {
			documentSelector: [{ scheme: 'file', language: 'bashpp' }],
			synchronize: {
				fileEvents: vscode.workspace.createFileSystemWatcher('**/*.bpp')
			}
		};

		client = new LanguageClient(
			'bashppLanguageServer',
			'Bash++ Language Server',
			serverOptions,
			clientOptions
		);
		context.subscriptions.push(client);
		client.start();
	}
}

async function activate(context) {
	console.log('Bash++ extension is activating...');
	// Register commands
	context.subscriptions.push(
		vscode.commands.registerCommand('bashpp.showOutput', () => {
			if (client) {
				client.outputChannel.show(true);
			} else {
				vscode.window.showErrorMessage('Bash++ Language Server is not running.');
			}
		}),

		vscode.commands.registerCommand('bashpp.enableLanguageServer', async () => {
			await vscode.workspace.getConfiguration('bashpp.languageServer')
				.update('enabled', true, vscode.ConfigurationTarget.Global);
			await startClient(context);
			vscode.window.showInformationMessage('Bash++ Language Server enabled.');
		}),

		vscode.commands.registerCommand('bashpp.disableLanguageServer', async () => {
			await vscode.workspace.getConfiguration('bashpp.languageServer')
				.update('enabled', false, vscode.ConfigurationTarget.Global);
			await stopClient();
			vscode.window.showInformationMessage('Bash++ Language Server disabled.');
		}),

		vscode.commands.registerCommand('bashpp.restartLanguageServer', async () => {
			await stopClient();
			await startClient(context);
			vscode.window.showInformationMessage('Bash++ Language Server restarted.');
		}),

		vscode.commands.registerCommand('bashpp.openSettings', () => {
			vscode.commands.executeCommand('workbench.action.openSettings', 'bashpp.languageServer');
		}),

		// Watch for configuration changes
		vscode.workspace.onDidChangeConfiguration(event => {
			if (event.affectsConfiguration('bashpp.languageServer')) {
				stopClient().then(() => startClient(context));
			}
		})
	);

	console.log('Bash++ extension commands registered.');

	await startClient(context);

	console.log('Bash++ Language Server started.');
}

function deactivate() {
	return stopClient();
}

module.exports = { activate, deactivate };
