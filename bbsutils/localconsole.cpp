#include "localconsole.h"
#include "ansiconsole.h"

namespace bbs {

#ifdef LINUX
LocalTerminal localTerminal;

Console *Console::createLocalConsole() {
	return new AnsiConsole(localTerminal);
}
#else
Console *Console::createLocalConsole() {
	return nullptr;
}
#endif

}