import json
from distutils.log import warn
from pprint import pprint

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
warn('***RAW DICT')
pprint(BOOKs)
warn('***PREETY PRINT DICT')
pprint(BOOKs)
warn('***RAW JSON')
pprint(json.dumps(BOOKs))
warn('***PRETTY PRINT JSON')
pprint(json.dumps(BOOKs,indent=4))