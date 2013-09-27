from cStringIO import StringIO

def handler(request):
  html = StringIO()
  html.write("<html><body><h1>Sent cookies:</h1>")

  try:
    cookies = request.headers["Cookie"].split(";")
    for cookie in cookies:
      name = cookie.split("=")[0]
      value = cookie.split("=")[1]
      html.write("<div class=\"cookie\">" + name + "=" + value + "</div>")
  except:
    pass

  html.write("</body></html>")

  request.send_response(200)
  request.send_header("Content-type", "text/html")
  request.send_header("Content-Length", html.tell())
  request.send_header("Cache-Control", "no-cache")
  request.end_headers()

  request.wfile.write(html.getvalue())
