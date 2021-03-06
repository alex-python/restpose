.. _searches:

========
Searches
========

A search is composed of a query, and various parameters to control which part
of the set of matching documents to return, which fields to return in the
matching documents, and which additional information about the matching
documents to calculate and return.

::

    SEARCH = {
        "query": QUERY,
        "from": <offset of first document to return.  Integer.  0 based.  Default=0>,
        "fromdoc": FROMDOC,
        "size": <maximum number of documents to return.  -1=return all matches.  Integer.  Default=10>,
        "check_at_least": <minimum number of documents to examine before early termination optimisations are allowed.  -1=check all matches.  Integer.  Default=0>,
        "info": [ INFO ],
        "order_by": ORDER_BY,
        "display": <list of fields to return>,
        "verbose": <flag indicating whether to return verbose debugging informat.  Boolean.  Default=false.>,
    }

Basic queries
=============

Matching everything, or nothing
-------------------------------

These seem a little pointless on first sight, but are useful placeholders for
slotting into a standard query structure, and occasionally have other uses.  In
many situations, these queries will be optimised away when performing the
search.

A query which matches all documents::

    QUERY = {
        "matchall": true
    }

A query which matches no documents::

    QUERY = {
        "matchnothing": true
    }

A search for a value in a particular field
------------------------------------------

::

    QUERY = {
        "field": [
            <fieldname>,
            <type of search to do>,
            <value to search for>
        ]
    }

Various types of search are possible:

 - "is": searches for documents in which a value stored in the field is equal
   to the value to search for.  This type is available for "exact", "id" and
   "category" field types.  The value to search for must be an array of values,
   each of which is either a string or an integer (in the range 0..2^64-1).

 - "is_descendant": searches for documents in which a value stored in the field
   is a descendant of a value specified in the search.  This type is available
   for "category" field types.  The value to search for must be an array of
   values, each of which is either a string or an integer (in the range
   0..2^64-1).

 - "is_or_is_descendant": searches for documents in which a value stored in the
   field is a value specified in the search, or is a descendant of a value
   specified in the search.  This type is available for "category" field types.
   The value to search for must be an array of values, each of which is either
   a string or an integer (in the range 0..2^64-1).

   This is produces an equivalent query to combining an "is" search with an
   "is_descendant" search using the "or" operator (though this version may be
   slightly faster to parse).

 - "range": searches for documents in which a stored value is in a given range.
   This type is currently available only for "double", "date" and "timestamp"
   field types.

 - "distscore": searches for documents within a given range of a center point,
   and returns a score which increases the closer a document is to that point.
   The value to search for must be an object holding the following parameters
   (or, at least, those parameters which are marked as required):

    - "center": Required.  The coordinate of the center point for the query;
      expressed either as a (longitude, latitude) pair, or as an object with
      "lon" and "lat" properties.
    - "max_range": Optional.  The maximum distance, in meters, that a
      document's coordinate may be from the center for that document to match
      the query.  Defaults to an unlimited distance.

 - "text": searches for a piece of text in a text field.  The value to search
   for may be a single string, or an object holding the following parameters:

   - "text": <text to search for.  If empty or missing, this query will match
     no results>

   - "op": <The operator to use when searching.  One of "or", "and", "phrase",
     "near".  Default=phrase>

   - "window": <only relevant if op is "phrase" or "near". window size in
     words; null=length of text. Integer or null. Default=null>

 - "parse": parses a query, and searches for the query in a text field.  The
   value to search for may be a single string, or an object holding the
   following parameters:

   - "text": <text to search for.  If empty or missing, this query will match
     no results>

   - "op": <The default operator to use when searching.  One of "or", "and".
     Default="and">

 - "exists": used on the meta field (by default, named `_meta`) to search for
   documents in which a field exists.  The value to search must be an array of
   values, each of which is either a fieldname to search for existence of, or
   "null" to search for existence of any field.  Note that the ID and type
   special fields are excluded from the meta field, so it is not possible to
   search for their existence.

 - "nonempty": used on the meta field (by default, named `_meta`) to search for
   documents in which a field exists and has a non-empty value.  The value to
   search must be an array of values, each of which is either a fieldname to
   search for non-empty values in, or "null" to search for non-empty values in
   any field.  Note that the ID and type special fields are excluded from the
   meta field, so it is not possible to search for nonempty values in them.

 - "empty": used on the meta field (by default, named `_meta`) to search for
   documents in which a field exists and has an empty value.  The value to
   search must be an array of values, each of which is either a fieldname to
   search for empty values in, or "null" to search for empty values in any
   field.  Note that the ID and type special fields are excluded from the meta
   field, so it is not possible to search for empty values in them.

 - "error": used on the meta field (by default, named `_meta`) to search for
   documents in which a field caused an error when processing.  The value to
   search must be an array of values, each of which is either a fieldname to
   search for error values in, or "null" to search for error values in any
   field.  Note that the ID and type special fields are excluded from the meta
   field, so it is not possible to search for error values in them.


Filtering results from another query
------------------------------------

The results from the primary query are returned, filtered so that only those
results which also match the filter are returned.

::

    QUERY = {
        "query": QUERY, <optional - defaults to matchall>
        "filter": QUERY
    }


Combining Queries
=================

::

    QUERY = {
        "and": [QUERY, ...]
    }

    QUERY = {
        "or": [QUERY, ...]
    }

    QUERY = {
        "xor": [QUERY, ...]
    }

    QUERY = {
        "and_not": [QUERY, ...]
    }

    QUERY = {
        "and_maybe": [QUERY, ...]
    }

Scale the weights returned by a query.
======================================

Weights of a query, at any point in the tree, can be scaled by multiplying them
by a constant factor.

::

    QUERY = {
        "scale": {
             "query": QUERY,
             "factor": <multiplier to apply to the weight.  Double, >= 0. Required.>
        }
    }

Getting additional information
==============================

Get co-occurrence counts for words in matching documents
--------------------------------------------------------

Warning - fairly slow (and O(L*L), where L is the average document length).

Returns counts for each pair of terms seen, in decreasing order of
cooccurrence.  The count entries are of the form: [suffix1, suffix2,
co-occurrence count] or [suffix1, suffix2, co-occurrence count, termfreq of
suffix1, termfreq of suffix2] if get_termfreqs was true.

::

    INFO = {
        "cooccur": {
            "prefix": <prefix of terms to check cooccurrence for>,
            "doc_limit": <number of matching documents to stop checking after.  null=unlimited.  Integer or null.  Default=null>
            "result_limit": <number of term pairs to return results for.  null=unlimited.  Integer or null. Default=null.>
            "get_termfreqs": <set to true to also get frequencies of terms in the db.  Boolean.  Default=false>
            "stopwords": <list of stopwords - term suffixes to ignore.  Array of strings.  Default=[]>
        }
    }

Getting term occurrence counts for words in matching documents
--------------------------------------------------------------

Warning - fairly slow.

Returns counts for each term seen, in decreasing order of occurrence.  The
count entries are of the form: [suffix, occurrence count] or [suffix,
occurrence count, termfreq] if get_termfreqs was true.

::

    INFO = {
        "occur": {
            "prefix": <prefix of terms to check occurrence for>,
            "doc_limit": <number of matching documents to stop checking after.  null=unlimited.  Integer or null.  Default=null>
            "result_limit": <number of terms to return results for.  null=unlimited.  Integer or null. Default=null.>
            "get_termfreqs": <set to true to also get frequencies of terms in the db.  Boolean.  Default=false>
            "stopwords": <list of stopwords - term suffixes to ignore.  Array of strings.  Default=[]>
        }
    }

Setting custom sort orders
==========================

By default, search results are ordered by a relevance score, calculated using
the BM25 weighting scheme.  The internal RestPose architecture allows for
considerable flexibility in how weights are calculated, and also allows for
ordering by schemes other than relevance score (eg, by a field value).  As yet,
little of this flexibility is exposed in the API, but more is planned to be.
Contact the author if you wish particular options to be made available.

Currently, the sort order can be set using the ``order_by`` configuration.  A sort order may be set using a field, as follows::

    ORDER_BY = [
        {"field": <field name>,
         "ascending": ASCENDING  // Optional - defaults to true
        }
    ]

    ASCENDING = <boolean - if true, the first results returned (ie, lowest rank) will have the lowest value for the field.>,


Alternately, the sort order can be set to be relevance order (which is the default order)::

    ORDER_BY = [
        {"score": "weight",
         "ascending": false  // Optional - defaults to false.  true is not allowed, but this is included for completeness.
        }
    ]

At present, the list of sort orders may only contain exactly one item.


Specifying a result set offset relative to a document
=====================================================

Sometimes, it's useful to be able to get the section of a search result set
containing a particular document, rather than just a section of a result set
based on an offset.  For example, imagine you're providing an interface which
allows users to page through a resultset where documents are constantly being
added to the underlying database, so additional documents may be added at any
time.

In this situation, it may be better to ask for the results which follow a
particular document, rather than to ask for the next page of results by a fixed
offset.  This is particularly true if documents are being returned sorted by
most-recent first; using this technique.

Note that this interface can also be used to get the rank of a given document
in a set of search results, without having to iterate through the search
results on the client side.

To get a set of search results based on the position of a document in the
search results, you can add a "fromdoc" property to the search::

    FROMDOC = {
        "type": <string: the document type of the base document>
        "id": <string: the document ID of the base document>
        "from": <integer: offset relative to the base document - may be negative.  Defaults to 0>
        "pagesize": <integer: number of results to calculate in each page when looking for the base document.  This may usually be ignored, but is available to allow tweaking for performance reasons.  Defaults to 10000.>
    }

Note that if a "fromdoc" property is supplied for a search, the "from" property must be 0 (or absent).

.. _search_results:

Search results
==============

Search results are returned as a JSON object, with the following properties.

 * ``from``: (int) The `from` value used when performing the search.

 * ``size_requested``: (int) The `size` value used when performing the search.

 * ``check_at_least``: (int) The `check_at_least` value used when performing
   the search.

 * ``total_docs``: (int) The total number of documents searched through.

 * ``matches_lower_bound``: (int) A lower bound on the number of matching
   documents.  This will be precise if `check_at_least` was -1, or was high
   enough to ensure that all matches were checked.

 * ``matches_estimated``: (int) An estimate on the number of matching
   documents.  This will be precise if `check_at_least` was -1, or was high
   enough to ensure that all matches were checked.

 * ``matches_upper_bound``: (int) An upper bound on the number of matching
   documents.  This will be precise if `check_at_least` was -1, or was high
   enough to ensure that all matches were checked.

 * ``items``: (array) An array of results from searching.  Each result is a
   object, keyed by fieldname, holding the stored fields for that result.  The
   search may limit which fields are returned.
