#include "parsing_table.h"

void ParsingTable::addAction(int state, const std::string& symbol, Action action) {
    // Implementation
}

Action ParsingTable::getAction(int state, const std::string& symbol) {
    // Implementation
    return {ERROR, 0};
}
