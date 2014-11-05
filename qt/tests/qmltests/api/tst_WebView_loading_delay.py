from cStringIO import StringIO
import time

def handler(request):
  time.sleep(0.5)

  html = StringIO()
  html.write("<html></html>")

  request.send_response(200)
  request.send_header("Content-Type", "text/html")
  request.send_header("Content-Length", html.tell())
  request.send_header("Cache-Control", "no-cache")
  request.end_headers()

  request.wfile.write(html.getvalue())
