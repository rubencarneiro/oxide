from cStringIO import StringIO
from datetime import datetime, timedelta

def handler(request):
  html = StringIO()
  html.write("<html></html>");

  expires = datetime.utcnow()
  expires += timedelta(hours=100)

  request.send_response(200)
  request.send_header("Content-Type", "application/pdf")
  request.send_header("Content-Length", html.tell())
  request.send_header("Cache-Control", "private")
  request.send_header("Set-Cookie", "foo=bar;Expires=%s;Domain=;Path=/" % expires.strftime("%a, %d-%b-%Y %H-%M-%S UTC"))
  request.end_headers()

  request.wfile.write(html.getvalue())
