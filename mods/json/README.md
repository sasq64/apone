

JSON
====
Simple json parsing. Uses json-c. Will throw exceptions if wrong datatype is used.

```C++

#include <json/json.h>

JSon json;
int trackid;
string name;
json.add("uid", trackid);
json.add("name", name);
json.add("version", API_VERSION);
auto sl = json.add_array("songs");
for(const SongInfo &info : songs) {
    sl.add(info.path);
}
```

```C++
    JSon json = JSon::parse(result);
    for(auto pl : json["urls"]) {
        list.emplace_back(pl.to_string());
    }
```