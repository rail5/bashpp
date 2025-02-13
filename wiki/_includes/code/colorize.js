const fs = require('fs');
const { Registry } = require('vscode-textmate');

const oniguruma = require('vscode-oniguruma');

const registry = new Registry({
  theme: {},
  loadGrammar: (scopeName) => {
    if (scopeName === 'source.bashpp') {
      return fs.promises.readFile('./bashpp.tmLanguage.json').then(data => {
        return JSON.parse(data);
      });
    }
    return null;
  },
  onigLib: oniguruma.loadWASM(fs.readFileSync(require.resolve('vscode-oniguruma/release/onig.wasm'))).then(() => oniguruma),
});

const args = process.argv.slice(2);
if (args.length < 2) {
  console.error('Usage: node colorize.js <input-file> <output-file>');
  process.exit(1);
}

const filePath = args[0];
const outputFilePath = args[1];

registry.loadGrammar('source.bashpp').then(grammar => {
  const fileContent = fs.readFileSync(filePath, 'utf-8');
  const lines = fileContent.split('\n');
  let ruleStack = null;
  const tokens = lines.map(line => {
    const result = grammar.tokenizeLine(line, ruleStack);
    ruleStack = result.ruleStack;
    return result.tokens;
  });
    const htmlLines = tokens.map((lineTokens, lineIndex) => {
      const lineContent = lines[lineIndex];
      const lineHtml = lineTokens.map(token => {
        const className = token.scopes.join(' ');
        const content = lineContent.substring(token.startIndex, token.endIndex);
        // Replace '<' and '>' with '&lt;' and '&gt;'
        const escapedContent = content.replace(/</g, '&lt;').replace(/>/g, '&gt;');
        return `<span class="${className}">${escapedContent}</span>`;
      }).join('');
      return lineHtml;
    }).join('\n');

    const htmlContent = `${htmlLines}`;
    fs.writeFileSync(outputFilePath, htmlContent);
  });
