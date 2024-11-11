#include <iostream>
#include <fstream>
#include <antlr4-runtime.h>
#include "out/BashppLexer.h"
#include "out/BashppParser.h"

using namespace antlr4;

int main(int argc, const char* argv[]) {
    if (argc < 2) {
        std::cerr << "Usage: " << argv[0] << " <input file>" << std::endl;
        return 1;
    }

    std::ifstream stream(argv[1]);
    if (!stream) {
        std::cerr << "Error: Could not open file " << argv[1] << std::endl;
        return 1;
    }

    ANTLRInputStream input(stream);
    BashppLexer lexer(&input);
    CommonTokenStream tokens(&lexer);

    // Print tokens and their types for debugging
    tokens.fill();
    for (auto token : tokens.getTokens()) {
        std::cout << "Token: " << token->getText() 
                  << ", Type: " << lexer.getVocabulary().getSymbolicName(token->getType()) 
                  << std::endl;
    }

    BashppParser parser(&tokens);

    // Remove default error listeners
    parser.removeErrorListeners();
    // Add diagnostic error listener
    parser.addErrorListener(new DiagnosticErrorListener());

    // Enable the parser to use diagnostic messages
    parser.setErrorHandler(std::make_shared<BailErrorStrategy>());

    tree::ParseTree* tree = nullptr;
    try {
        tree = parser.program();
        std::cout << "Parse tree: " << std::endl;
        std::cout << tree->toStringTree(&parser) << std::endl;
    } catch (const antlr4::EmptyStackException& e) {
        std::cerr << "EmptyStackException: " << e.what() << std::endl;
    } catch (const RecognitionException& e) {
        std::cerr << "Recognition exception: " << e.what() << std::endl;
    } catch (const std::exception& e) {
        std::cerr << "Standard exception: " << e.what() << std::endl;
    } catch (...) {
        std::cerr << "Unknown exception occurred" << std::endl;
    }

    return 0;
}