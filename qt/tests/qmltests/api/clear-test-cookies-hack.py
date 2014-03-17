from cStringIO import StringIO

# FIXME(chrisccoulson): We desperately need an API for clearing site data
# (not just cookies). This abomination is for ensuring there are no test
# cookies left from a previous test. Yeah, it sucks....

def handler(request):
  html = StringIO()
  html.write("<html></html>")

  request.send_response(200)
  request.send_header("Content-type", "text/html")
  request.send_header("Content-Length", html.tell())
  request.send_header("Cache-Control", "no-cache")

  if "Cookie" in request.headers:
    cookies = request.headers["Cookie"].split(";")
    for cookie in cookies:
      request.send_header("Set-Cookie", "%s=deleted;Expires=Thu, 01-01-1970 00-00-00 GMT;Domain=;Path=/" % cookie.split("=")[0])

  request.end_headers()

  request.wfile.write(html.getvalue())
