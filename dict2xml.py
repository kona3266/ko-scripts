from xml.etree.ElementTree import Element, SubElement, tostring
from xml.dom.minidom import parseString

BOOKs = {
    '0132269937': {
        'title': 'Core Python Programming',
        'edition': 2,
        'year': 2007,
    },
    '0132356139': {
        'title': 'Python Web Development with Django',
        'authors': ['Jeff Forcier', 'Paul Bissex', 'Wesley Chun'],
        'year': 2009
    },
    '0.17143419': {
        'title': 'Python Fundamentals',
        'year': 2009
    },
}

books = Element('books')
for isbn, infos in BOOKs.items():
    book = SubElement(books, 'book')
    for key, val in infos.items():
        SubElement(book, key).text = ', '.join(str(val).split(":"))
xml = tostring(books)
print('*** RAW XML')
print(xml)
print('***PRETTY PRINT XML')
dom = parseString(xml)
print(dom.toprettyxml('    '))
print('*** FLAT STRUCTURE')
for ele in books.iter():
    print(ele.tag, '-', ele.text)
print('***TITLES ONLY')
for book in books.findall('.//title'):
    print(book.text)
