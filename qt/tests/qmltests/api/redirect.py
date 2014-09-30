from cStringIO import StringIO

def handler(request):
  html = StringIO()
  html.write("<html></html>");

  request.send_response(307)
  request.send_header("Content-type", "test/html")
  request.send_header("Content-Length", html.tell())
  request.send_header("Location", "http://localhost:8080/empty.html")
  request.send_header("Cache-Control", "no-cache")
  request.end_headers()

  request.wfile.write(html.getvalue())
