from cStringIO import StringIO

def handler(request):
  html = StringIO()
  html.write("{");

  first = True
  for header in request.headers:
    val = request.headers[header]
    if not first:
      html.write(",")
    first = False
    html.write("\"%s\": \"%s\"" % (header, val))

  html.write("}")

  request.send_response(200)
  request.send_header("Content-type", "application/json")
  request.send_header("Content-Length", html.tell())
  request.send_header("Cache-Control", "no-cache")
  request.end_headers()

  request.wfile.write(html.getvalue())
