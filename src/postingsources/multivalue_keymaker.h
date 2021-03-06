/** @file multivalue_keymaker.h
 * @brief KeyMaker for sorting by multivalued slots
 */
/* Copyright (c) 2011 Richard Boulton
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to
 * deal in the Software without restriction, including without limitation the
 * rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
 * sell copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
 * IN THE SOFTWARE.
 */

#ifndef RESTPOSE_INCLUDED_MULTIVALUE_KEYMAKER_H
#define RESTPOSE_INCLUDED_MULTIVALUE_KEYMAKER_H

#include <xapian.h>
#include <vector>

namespace RestPose {
    class SlotDecoder;

    class MultiValueKeyMaker : public Xapian::KeyMaker {
	std::vector<std::pair<SlotDecoder *, bool> > decoders;
      public:
	MultiValueKeyMaker() : decoders() {}
	void add_decoder(SlotDecoder * decoder, bool reverse = false);
	std::string operator()(const Xapian::Document & doc) const;
    };

}

#endif /* RESTPOSE_INCLUDED_MULTIVALUE_KEYMAKER_H */
