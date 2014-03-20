from cStringIO import StringIO

def handler(request):
  html = StringIO()
  html.write("<html><body><h1>Sent Headers:</h1>")

  for h in [ "User-Agent", "Foo" ]:
    try:
      val = request.headers[h]
      html.write("<div class=\"header\">" + h + "=" + val + "</div>")
    except:
      pass

  html.write("</body></html>")

  request.send_response(200)
  request.send_header("Content-type", "text/html")
  request.send_header("Content-Length", html.tell())
  request.send_header("Cache-Control", "no-cache")
  request.end_headers()

  request.wfile.write(html.getvalue())
