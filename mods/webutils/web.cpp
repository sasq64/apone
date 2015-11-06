
#include "web.h"


namespace webutils {

//std::vector<std::shared_ptr<Web::Job>> Web::jobs;
int Web::totalRunning = 0;
std::mutex Web::sm;
bool Web::initDone = false;
} // namespace webutils

