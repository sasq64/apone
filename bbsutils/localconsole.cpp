#include "localconsole.h"
#include "ansiconsole.h"

namespace bbs {

LocalTerminal localTerminal;

Console *Console::createLocalConsole() {
	return new AnsiConsole(localTerminal);
}

}