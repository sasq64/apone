#include <emscripten.h>


static SongDatabase db;
static IncrementalQuery query;


void init_db() {
	db.generateIndex();
	query = db.find();
}

void set_string(char *str, int size) {
	query.setString(s);
}



void get_results(char *target, int size) {

	const auto &results = query.getResult(start, count);
	int i=0;
	for(const auto &r : results) {
		strcpy(target, r);
		target += (r.length()+1);
	}
};