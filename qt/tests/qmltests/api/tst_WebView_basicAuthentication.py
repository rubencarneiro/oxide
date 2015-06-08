from cStringIO import StringIO
import base64

def handler(request):
  html = StringIO()

  base = len("/tst_BasicAuthentication.py") + 1
  query = request.path[base:]
  if "_" in query:
      user, password = query.split("_")

  def send_401():
      request.send_response(401)
      request.send_header("WWW-Authenticate", "Basic realm=\"Fake Realm\"")
      request.send_header("Cache-Control", "no-store")
      request.send_header("Content-Length", html.tell())
      request.end_headers()

  if "Authorization" in request.headers:
      header = request.headers["Authorization"]
      credentials = base64.b64decode(header[len("Basic "):]).split(":")
      if credentials[0] == user and credentials[1] == password:
          html.write("Authentication success")
          request.send_response(200)
          request.send_header("Cache-Control", "no-store")
          request.send_header("Content-Length", html.tell())
          request.end_headers()
      else:
          html.write("Wrong credentials")
          send_401()
  else:
    html.write("Requested credentials")
    send_401()

  request.wfile.write(html.getvalue())
