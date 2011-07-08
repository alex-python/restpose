# -*- coding: utf-8 -
#
# This file is part of the restpose python module, released under the MIT
# license.  See the COPYING file for more information.

from unittest import TestCase
from .. import query, Server

class SearchTest(TestCase):
    maxDiff = 10000

    # Expected items for tests which return a single result
    expected_items_single = [
        query.SearchResult(rank=0, fields={
                'cat': ['greeting'],
                'empty': [''],
                'id': ['1'],
                'tag': ['A tag'],
                'text': ['Hello world'],
                'type': ['blurb'],
            }),
        ]

    def check_results(self, results, offset=0, size=10, check_at_least=0,
                      matches_lower_bound=None,
                      matches_estimated=None,
                      matches_upper_bound=None,
                      items = [],
                      info = {}):
        if matches_lower_bound is None:
            matches_lower_bound = len(items)
        if matches_estimated is None:
            matches_estimated = len(items)
        if matches_upper_bound is None:
            matches_upper_bound = len(items)

        self.assertEqual(results.offset, offset)
        self.assertEqual(results.size, size)
        self.assertEqual(results.check_at_least, check_at_least)
        self.assertEqual(results.matches_lower_bound, matches_lower_bound)
        self.assertEqual(results.matches_estimated, matches_estimated)
        self.assertEqual(results.matches_upper_bound, matches_upper_bound)
        self.assertEqual(len(results.items), len(items))
        for i in xrange(len(items)):
            self.assertEqual(results.items[i].rank, items[i].rank)
            self.assertEqual(results.items[i].fields, items[i].fields)
        self.assertEqual(results.info, info)

    @classmethod
    def setup_class(cls):
        doc = { 'text': 'Hello world', 'tag': 'A tag', 'cat': "greeting",
                'empty': "" }
        coll = Server().collection("test_coll")
        #coll.delete()
        coll.add_doc(doc, type="blurb", id="1")
        coll.checkpoint().wait()

    def setUp(self):
        self.coll = Server().collection("test_coll")

    def test_indexed_ok(self):
        """Check that setup put the database into the desired state.

        """
        self.assertEqual(self.coll.status.get('doc_count'), 1)
        gotdoc = self.coll.get_doc("blurb", "1")
        self.assertEqual(gotdoc, dict(data={
                                      'cat': ['greeting'],
                                      'empty': [''],
                                      'id': ['1'],
                                      'tag': ['A tag'],
                                      'text': ['Hello world'],
                                      'type': ['blurb'],
                                      }, terms={
                                      '\tblurb\t1': {},
                                      '!\tblurb': {},
                                      '#\tFcat': {},
                                      '#\tFempty': {},
                                      '#\tFtag': {},
                                      '#\tFtext': {},
                                      '#\tM': {},
                                      '#\tMempty': {},
                                      '#\tN': {},
                                      '#\tNcat': {},
                                      '#\tNtag': {},
                                      '#\tNtext': {},
                                      'Zt\thello': {'wdf': 1},
                                      'Zt\tworld': {'wdf': 1},
                                      'c\tCgreeting': {},
                                      'g\tA tag': {},
                                      't\thello': {'positions': [1], 'wdf': 1},
                                      't\tworld': {'positions': [2], 'wdf': 1},
                                      }))

    def test_field_is(self):
        q = self.coll.type("blurb").field_is('tag', 'A tag')
        results = q.search().do()
        self.check_results(results, items=self.expected_items_single)

    def test_field_exists(self):
        q = self.coll.type("blurb").field_exists()
        results = q.search().do()
        self.check_results(results, items=self.expected_items_single)

        q = self.coll.type("blurb").field_exists('tag')
        results = q.search().do()
        self.check_results(results, items=self.expected_items_single)

        q = self.coll.type("blurb").field_exists('id')
        # ID field is not stored, so searching for its existence returns
        # nothing.
        results = q.search().do()
        self.check_results(results, items=[])

        q = self.coll.type("blurb").field_exists('type')
        # Type field is not stored, so searching for its existence returns
        # nothing.
        results = q.search().do()
        self.check_results(results, items=[])

        q = self.coll.type("blurb").field_exists('missing')
        results = q.search().do()
        self.check_results(results, items=[])

    def test_field_empty(self):
        q = self.coll.type("blurb").field_empty()
        self.check_results(q.search().do(), items=self.expected_items_single)

        q = self.coll.type("blurb").field_empty('empty')
        self.check_results(q.search().do(), items=self.expected_items_single)

        q = self.coll.type("blurb").field_empty('text')
        self.check_results(q.search().do(), items=[])

        q = self.coll.type("blurb").field_empty('id')
        self.check_results(q.search().do(), items=[])

        q = self.coll.type("blurb").field_empty('type')
        self.check_results(q.search().do(), items=[])

        q = self.coll.type("blurb").field_empty('missing')
        self.check_results(q.search().do(), items=[])

    def test_field_nonempty(self):
        q = self.coll.type("blurb").field_nonempty()
        self.check_results(q.search().do(), items=self.expected_items_single)

        q = self.coll.type("blurb").field_nonempty('empty')
        self.check_results(q.search().do(), items=[])

        q = self.coll.type("blurb").field_nonempty('text')
        self.check_results(q.search().do(), items=self.expected_items_single)

        q = self.coll.type("blurb").field_nonempty('id')
        self.check_results(q.search().do(), items=[])

        q = self.coll.type("blurb").field_nonempty('type')
        self.check_results(q.search().do(), items=[])

        q = self.coll.type("blurb").field_nonempty('missing')
        self.check_results(q.search().do(), items=[])

    def test_field_has_error(self):
        q = self.coll.type("blurb").field_has_error()
        self.check_results(q.search().do(), items=[])


    def test_calc_cooccur(self):
        q = self.coll.type("blurb").query_all()
        s = q.search()
        results = q.search().calc_cooccur('t').do()
        self.assertEqual(self.coll.status.get('doc_count'), 1)
        self.check_results(results, check_at_least=1,
                           items=self.expected_items_single,
                           info=[{
                               'counts': [['hello', 'world', 1]],
                               'docs_seen': 1,
                               'terms_seen': 2,
                               'prefix': 't',
                               'type': 'cooccur'
                           }])

    def test_calc_occur(self):
        q = self.coll.type("blurb").query_all()
        s = q.search()
        results = q.search().calc_occur('t').do()
        self.assertEqual(self.coll.status.get('doc_count'), 1)
        self.check_results(results, check_at_least=1,
                           items=self.expected_items_single,
                           info=[{
                               'counts': [['hello', 1], ['world', 1]],
                               'docs_seen': 1,
                               'terms_seen': 2,
                               'prefix': 't',
                               'type': 'occur'
                           }])

    def test_raw_query(self):
        results = self.coll.type("blurb").search(dict(query=dict(matchall=True)))
        self.check_results(results, items=self.expected_items_single)
