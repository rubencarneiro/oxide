from cStringIO import StringIO
import re

def handler(request):
  html = StringIO()
  html.write("<html></html>");

  request.send_response(307)
  request.send_header("Content-type", "text/html")
  request.send_header("Content-Length", html.tell())
  request.send_header("Location", re.match(r"[^\?]+\??(.*)", request.path).groups()[0])
  request.send_header("Cache-Control", "no-cache")
  request.end_headers()

  request.wfile.write(html.getvalue())
