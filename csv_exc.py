import csv
from distutils.log import warn
DATA = (
    (9, 'web clients', 'base64, urllib'),
    (10, 'web programming: CGI & WSGI', 'cgi, time, wsgiref'),
    (13, 'web services', 'urllib, twython')
)
warn('***Writing CSV DATA')
f = open('data.csv', 'w', newline='')
writer = csv.writer(f)
for record in DATA:
    warn(str(record))
    writer.writerow(record)
f.close()
warn('***Reading CSV DATA')
f = open('data.csv', 'r')
rows = csv.reader(f)
for row in rows:
    warn('{0}&&{1}&&{2}'.format(chap, title, modpkgs))
f.close()