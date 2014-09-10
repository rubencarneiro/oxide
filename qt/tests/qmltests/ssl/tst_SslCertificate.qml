import QtQuick 2.0
import QtTest 1.0
import com.canonical.Oxide 1.0
import com.canonical.Oxide.Testing 1.0

TestWebView {
  id: webView

  width: 200
  height: 200

  onCertificateError: {
    error.allow();
  }

  TestCase {
    id: test
    name: "SslCertificate"
    when: windowShown

    property var cert_data: [
      { id: 1, parent: 5,
        serial: "00863486245ae549b6", startDate: 13054686690000, expiryDate: 21694600290000,
        fingerprint: "b354a8e3d1359447ec719e7a03b42cef379a4cc1", expired: false,
        subject: {O: "Oxide", CN: "localhost", L: "Solihull", OU: undefined, C: "UK", ST: "West Midlands"},
        issuer: {O: "Canonical Ltd", CN: "Oxide Root CA", L: "Solihull", OU: undefined, C: "UK", ST: "West Midlands"}
      },
      { id: 2, parent: null,
        serial: "00efc9a8e1a7a14969", startDate: 13023211916000, expiryDate: 13023298316000,
        fingerprint: "df17c8da033e2d5ed64d1f187fdf419e1fc68e47", expired: true,
        subject: {O: "Oxide", CN: "localhost", L: "Solihull", OU: undefined, C: "UK", ST: "West Midlands"},
        issuer: {O: "Canonical Ltd", CN: "Oxide Root CA", L: "Solihull", OU: undefined, C: "UK", ST: "West Midlands"}
      },
      { id: 3, parent: null,
        serial: "00d8b91a89244cc2bd", startDate: 13054749416000, expiryDate: 21694663016000,
        fingerprint: "f0357f544e27adaa51211663a28cc8d64b057e63", expired: false,
        subject: {O: "Oxide", CN: "localhost", L: "Solihull", OU: undefined, C: "UK", ST: "West Midlands"},
        issuer: {O: "Oxide", CN: "localhost", L: "Solihull", OU: undefined, C: "UK", ST: "West Midlands"}
      },
      { id: 4, parent: null,
        serial: "00b3ebd987ee2d02cc", startDate: 13054749837000, expiryDate: 21694663437000,
        fingerprint: "89c5760286e897ad32b9dd500d70755e1e026588", expired: false,
        subject: {O: "Oxide", CN: "foo.bar", L: "Solihull", OU: undefined, C: "UK", ST: "West Midlands"},
        issuer: {O: "Canonical Ltd", CN: "Oxide Root CA", L: "Solihull", OU: undefined, C: "UK", ST: "West Midlands"}
      },
      { id: 5, parent: null,
        serial: "00b2e64281f82a21c0", startDate: 13054686336000, expiryDate: 21694599936000,
        fingerprint: "f8ad76468322c78acbeb0845b3cdbe07138f32c4", expired: false,
        subject: {O: "Canonical Ltd", CN: "Oxide Root CA", L: "Solihull", OU: undefined, C: "UK", ST: "West Midlands"},
        issuer: {O: "Canonical Ltd", CN: "Oxide Root CA", L: "Solihull", OU: undefined, C: "UK", ST: "West Midlands"}
      }
    ]

    function cert_data_from_id(id) {
      if (id === null) {
        return null;
      }

      for (var i = 0; i < cert_data.length; ++i) {
        if (cert_data[i].id == id) {
          return cert_data[i];
        }
      }

      return null;
    }

    function test_SslCertificate1_data() {
      return [
        { url: "https://localhost:4443/empty.html", cert: 1 },
        { url: "https://localhost:4444/empty.html", cert: 2 },
        { url: "https://localhost:4445/empty.html", cert: 3 },
        { url: "https://localhost:4446/empty.html", cert: 4 }
      ];
    }

    function test_SslCertificate1(data) {
      webView.url = data.url;
      verify(webView.waitForLoadSucceeded());

      var certificate = webView.securityStatus.certificate;
      var cdata = cert_data_from_id(data.cert);

      while (cdata) {
        compare(certificate.serialNumber, cdata.serial);
        compare(certificate.effectiveDate.getTime(), cdata.startDate);
        compare(certificate.expiryDate.getTime(), cdata.expiryDate);
        compare(certificate.fingerprintSHA1, cdata.fingerprint);
        compare(certificate.isExpired, cdata.expired);

        compare(certificate.getSubjectInfo(SslCertificate.PrincipalAttrOrganizationName)[0], cdata.subject.O);
        compare(certificate.getSubjectInfo(SslCertificate.PrincipalAttrCommonName)[0], cdata.subject.CN);
        compare(certificate.getSubjectInfo(SslCertificate.PrincipalAttrLocalityName)[0], cdata.subject.L);
        compare(certificate.getSubjectInfo(SslCertificate.PrincipalAttrOrganizationUnitName)[0], cdata.subject.OU);
        compare(certificate.getSubjectInfo(SslCertificate.PrincipalAttrCountryName)[0], cdata.subject.C);
        compare(certificate.getSubjectInfo(SslCertificate.PrincipalAttrStateOrProvinceName)[0], cdata.subject.ST);

        compare(certificate.getIssuerInfo(SslCertificate.PrincipalAttrOrganizationName)[0], cdata.issuer.O);
        compare(certificate.getIssuerInfo(SslCertificate.PrincipalAttrCommonName)[0], cdata.issuer.CN);
        compare(certificate.getIssuerInfo(SslCertificate.PrincipalAttrLocalityName)[0], cdata.issuer.L);
        compare(certificate.getIssuerInfo(SslCertificate.PrincipalAttrOrganizationUnitName)[0], cdata.issuer.OU);
        compare(certificate.getIssuerInfo(SslCertificate.PrincipalAttrCountryName)[0], cdata.issuer.C);
        compare(certificate.getIssuerInfo(SslCertificate.PrincipalAttrStateOrProvinceName)[0], cdata.issuer.ST);

        certificate = certificate.issuer;
        cdata = cert_data_from_id(cdata.parent);
      }

      verify(!certificate);
    }
  }
}
