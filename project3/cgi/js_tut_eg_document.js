function newpage(count) {
count++
var output='<html><head><title>' + count + '</title>'+
    '<script src="js_tut_eg_document.js"></script>'+
    '</head><body>'+ count +
    '<form><input type=button value="New Page" '+
    'onclick="newpage(' + count + ')"></form>'+
    '</body></html>'
document.close()
document.write(output)
}