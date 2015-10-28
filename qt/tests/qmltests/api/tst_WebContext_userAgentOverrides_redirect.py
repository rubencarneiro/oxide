from cStringIO import StringIO

def handler(request):
  html = StringIO()
  html.write("<html></html>");

  request.send_response(302)
  request.send_header("Content-type", "text/html")
  request.send_header("Content-Length", html.tell())
  request.send_header("Location", "http://2.testsuite/get-headers.py")
  request.send_header("Cache-Control", "no-cache")
  request.end_headers()

  request.wfile.write(html.getvalue())

