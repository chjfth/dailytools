// [2022-04-09] Used by:
// https://www.evernote.com/l/ABWtN6AaxKhLUpV4YoVxZgYnBxdbgnDy-jg/

var ims=document.getElementsByTagName('img');
for(var im of ims) {
	var altext = im.alt;
	var m = altext.match(/max-width:\s*([0-9]+)px/);
	if(m) {
		im.style.maxWidth = m[1] + "px";
		im.style.width = "100%";
	}
}
