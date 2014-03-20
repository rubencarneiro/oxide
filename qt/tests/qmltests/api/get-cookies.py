from cStringIO import StringIO

def handler(request):
  html = StringIO()
  html.write("{");

  if "Cookie" in request.headers:
    cookies = request.headers["Cookie"].split(";")
    first = True
    for cookie in cookies:
      if not first:
        html.write(",")
      first = False
      html.write("\"%s\": \"%s\"" % (cookie.split("=")[0], cookie.split("=")[1]))

  html.write("}")

  request.send_response(200)
  request.send_header("Content-type", "application/json")
  request.send_header("Content-Length", html.tell())
  request.send_header("Cache-Control", "no-cache")
  request.end_headers()

  request.wfile.write(html.getvalue())
