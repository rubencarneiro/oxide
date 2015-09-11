from cStringIO import StringIO

def handler(request):
  request.send_response(200)
  request.send_header("Content-type", "text/html")

  html = StringIO()
  for header in request.headers:
    print header
  html.write(
      "<html><body><div id='dnt'>{}</div></body></html>".format(
          "1" if 'DNT' in request.headers and request.headers['DNT'] == '1' else "0"))

  request.send_header("Content-Length", html.tell())
  request.send_header("Cache-Control", "no-cache")
  request.end_headers()

  request.wfile.write(html.getvalue())
