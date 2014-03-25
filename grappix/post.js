
playsid_play = function(name, done_cb, progress_cb) {
	var done_num = Runtime.addFunction(function(title, composer, copyright, max_song, start_song) {
		var o = {};
		o.title = Pointer_stringify(title);
		o.composer = Pointer_stringify(composer);
		o.copyright = Pointer_stringify(copyright);
		o.songs = max_song;
		o.start_song = start_song+1;
		done_cb(o);
		Runtime.removeFunction(done_num);
	});
	//var progress_num = Runtime.addFunction(progress_cb);
	Module.ccall('playsid_play', 'void', ['string', 'number', 'number'], [name, done_num, 0]);
}

playsid_pause = function(dopause) {
	Module.ccall('playsid_pause', 'void', ['number'], [dopause]);
}

playsid_setsong = function(song) {
	Module.ccall('playsid_setsong', 'void', ['number'], [song-1]);
}