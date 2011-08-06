/** @file category_tasks.cc
 * @brief Tasks related to categories.
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

#include <config.h>
#include "features/category_tasks.h"

#include "httpserver/response.h"
#include "jsonxapian/collection.h"
#include "logger/logger.h"
#include "server/task_manager.h"
#include "utils/jsonutils.h"
#include "utils/stringutils.h"

using namespace RestPose;
using namespace std;

void
CollGetCategoryHierarchiesTask::perform(Collection * coll)
{
    Json::Value result;
    coll->get_category_hierarchy_names(result);
    resulthandle.response().set(result, 200);
    resulthandle.set_ready();
}

void
CollGetCategoryHierarchyTask::perform(Collection * coll)
{
    Json::Value result(Json::objectValue);
    const CategoryHierarchy * hier
	    = coll->get_category_hierarchy(hierarchy_name);
    if (hier == NULL) {
	result["err"] = "Category hierarchy \"" + hexesc(hierarchy_name)
		+ "\" not found";
	resulthandle.response().set(result, 404);
	resulthandle.set_ready();
	return;
    }
    hier->to_json(result);

    resulthandle.response().set(result, 200);
    resulthandle.set_ready();
}

void
CollGetCategoryTask::perform(Collection * coll)
{
    Json::Value result(Json::objectValue);
    const CategoryHierarchy * hier
	    = coll->get_category_hierarchy(hierarchy_name);
    if (hier == NULL) {
	result["err"] = "Category hierarchy \"" + hexesc(hierarchy_name)
		+ "\" not found";
	resulthandle.response().set(result, 404);
	resulthandle.set_ready();
	return;
    }

    const Category * cat = hier->find(cat_id);
    if (cat == NULL) {
	result["err"] = "Category \"" + hexesc(cat_id) + "\" not found";
	resulthandle.response().set(result, 404);
	resulthandle.set_ready();
	return;
    }

    Json::Value & parents = result["parents"] = Json::arrayValue;
    for (Categories::const_iterator i = cat->parents.begin();
	 i != cat->parents.end(); ++i) {
	parents.append(*i);
    }

    Json::Value & children = result["children"] = Json::arrayValue;
    for (Categories::const_iterator i = cat->children.begin();
	 i != cat->children.end(); ++i) {
	children.append(*i);
    }

    Json::Value & ancestors = result["ancestors"] = Json::arrayValue;
    for (Categories::const_iterator i = cat->ancestors.begin();
	 i != cat->ancestors.end(); ++i) {
	ancestors.append(*i);
    }

    Json::Value & descendants = result["descendants"] = Json::arrayValue;
    for (Categories::const_iterator i = cat->descendants.begin();
	 i != cat->descendants.end(); ++i) {
	descendants.append(*i);
    }

    resulthandle.response().set(result, 200);
    resulthandle.set_ready();
}

void
CollGetCategoryParentTask::perform(Collection * coll)
{
    Json::Value result(Json::objectValue);
    const CategoryHierarchy * hier
	    = coll->get_category_hierarchy(hierarchy_name);
    if (hier == NULL) {
	result["err"] = "Category hierarchy \"" + hexesc(hierarchy_name)
		+ "\" not found";
	resulthandle.response().set(result, 404);
	resulthandle.set_ready();
    }

    const Category * cat = hier->find(cat_id);
    if (cat == NULL) {
	result["err"] = "Category \"" + hexesc(cat_id) + "\" not found";
	resulthandle.response().set(result, 404);
	resulthandle.set_ready();
	return;
    }

    if (cat->parents.find(parent_id) == cat->parents.end()) {
	result["err"] = "Category \"" + hexesc(parent_id)
		+ "\" not a parent of \"" + hexesc(parent_id) + "\"";
	resulthandle.response().set(result, 404);
	resulthandle.set_ready();
	return;
    }

    resulthandle.response().set(result, 200);
    resulthandle.set_ready();
}

void
ProcessingCollPutCategoryParentTask::perform(const std::string & coll_name,
					     TaskManager * taskman)
{
    LOG_INFO("PutCategoryParentTask:" + coll_name + "," + hierarchy_name +
	     "," + cat_id + "," + parent_id);
    auto_ptr<CollectionConfig> collconfig(taskman->get_collconfigs()
					  .get(coll_name));
    Json::Value tmp;
    LOG_INFO(json_serialise(collconfig->to_json(tmp)));
    {
	Categories modified;
	(void)collconfig->category_add_parent(hierarchy_name, cat_id,
					      parent_id, modified);
    }
    LOG_INFO(json_serialise(collconfig->to_json(tmp)));
    taskman->get_collconfigs().set(coll_name, collconfig.release());
    taskman->queue_indexing_from_processing(coll_name,
	new CollPutCategoryParentTask(hierarchy_name, cat_id, parent_id));
}

void
CollPutCategoryParentTask::perform_task(const std::string & coll_name,
					RestPose::Collection * & collection,
					TaskManager * taskman)
{
    if (collection == NULL) {
	collection = taskman->get_collections().get_writable(coll_name);
    }
    collection->category_add_parent(hierarchy_name, cat_id, parent_id);
    Json::Value tmp;
    LOG_INFO(json_serialise(collection->to_json(tmp)));
}

void
CollPutCategoryParentTask::info(std::string & description,
				std::string & doc_type,
				std::string & doc_id) const
{
    description = "Adding category parent";
    doc_type.resize(0);
    doc_id.resize(0);
}

IndexingTask *
CollPutCategoryParentTask::clone() const
{
    return new CollPutCategoryParentTask(hierarchy_name, cat_id, parent_id);
}