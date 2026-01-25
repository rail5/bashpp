const vscode = require('vscode');
const { LanguageClient, TransportKind } = require('vscode-languageclient');

/** @type {LanguageClient | null} */
let client = null;

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
		const targetBashVersion = config.get('targetBashVersion', '5.2');
		args.push(`-b${targetBashVersion}`);

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
