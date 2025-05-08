// Copyright 2025 The Dawn & Tint Authors
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//
// 1. Redistributions of source code must retain the above copyright notice, this
//    list of conditions and the following disclaimer.
//
// 2. Redistributions in binary form must reproduce the above copyright notice,
//    this list of conditions and the following disclaimer in the documentation
//    and/or other materials provided with the distribution.
//
// 3. Neither the name of the copyright holder nor the names of its
//    contributors may be used to endorse or promote products derived from
//    this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
// DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
// FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
// DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
// SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
// CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
// OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.


// Controller for the Dawn failure expectations browser.


const width = 700;
const height = 700;

// The application state.
// The UI sets configuration parameters and callbacks.
// The application logic populates values from the file.
var Model = {
  file: undefined,
  file_lines: undefined,

  // Derived from file data
  num_leaves: 1, // updated on reread
  counted_data: undefined, // treemap data for failure counts by path
  counted_tags: undefined, // treemap data for tag counts
  live_tags: new Set(), // The set of leaf tags that are currently valid
  treemap_id_to_node: undefined,
  treemap_svg_node: undefined,

  // Dependent on file data.
  focus_tags: [], // only show items with these tags. Acts a stack
  path_prefix: '', // path prefix for all shown items

  // Derived from UI controls
  path_substring: '',
  max_levels: 4, // Level of detail
  show_skip: true,
  show_fail: true,
  show_retry: true,
  show_slow: true,
  show_with_bug: true,
  show_without_bug: true,


  // Callbacks that control the UI.

  // The function that will display numbered text lines.
  // e.g. numbered_lines_displayer([{lineNumber:1,line:"foo"}, {lineNumber:16,line:"bar"}])
  numbered_lines_displayer: (list_of_number_and_text) => {},

  detail_incrementer: () => {},
  node_name_displayer: (str,index) => {},
  tag_name_displayer: (str,index) => {},
};

function resetFileDerivedData() {

  Model.num_leaves = 1;
  Model.counted_data = undefined;
  Model.counted_tags = undefined;
  Model.live_tags = new Set();
  Model.treemap_id_to_node = undefined;
  Model.treemap_svg_node = undefined;

  Model.focus_tags = [];
  Model.path_prefix = '';

  // Clear outputs and controls
  Model.node_name_displayer('',0); // clear the path control
  Model.node_name_displayer('',1);
  Model.node_name_displayer('',2);
  Model.tag_name_displayer('',0);
  Model.tag_name_displayer('',1);
  Model.tag_name_displayer('',2);
  Model.numbered_lines_displayer([]);
}
resetFileDerivedData();


// Unique IDs, for elements and clip paths.
var count = 0;

function uid(name) {
  return new Id("O-" + (name == null ? "" : name + "-") + ++count);
}

function Id(id) {
  this.id = id;
  this.href = new URL(`#${id}`, location) + "";
}

Id.prototype.toString = function() {
  return "url(" + this.href + ")";
};


// Treemap generation, for visualizing hierarchical test counts.
function makeTreemap(counted_hierarchy) {
  const sorted = counted_hierarchy.sort((a, b) => b.height - a.height || b.value - a.value);
  const data = sorted;
  Model.num_leaves = data.value;

  // Specify the color scale.
  // Protect against the case where there is no data at all.
  const color = d3.scaleOrdinal((data.children ?? []).map(d => d.data.id), d3.schemeTableau10);

  // Compute the layout.
  const root = d3.treemap()
    .tile(d3.treemapSquarify) // e.g., d3.treemapSquarify
    .size([width, height])
    .padding(0)
    .round(true)
  (data);

  // Create the SVG container.
  const svg = d3.create("svg")
      .attr("viewBox", [0, 0, width, height])
      .attr("width", width)
      .attr("height", height)
      .attr("style", `min-width: 30%;
                      max-width: 100%;
                      height: auto;
                      font: 15px sans-serif;
                      border: 1px solid black`);

  // Add a cell for each node of the hierarchy.
  const cell = svg.selectAll("g")
    .data(root.descendants())
    .join("g");

  const format5 = d3.format("5,d");
  const format = d3.format(",d");
  const percent_format = d3.format(".2%");
  function showNodeText(d) {
    if (Model.node_name_displayer) {
      Model.node_name_displayer(d.data.id??'',1);
      Model.node_name_displayer(`${format5(d.value)} / ${Model.num_leaves} = ${percent_format(d.value / Model.num_leaves)}`,2);
    }
  }

  cell.on("click", (e,target) => {
    if (e.shiftKey || e.ctrlKey || e.altKey || e.metaKey) {
      handleSetPathPrefixOut();
    } else {
      handleSetPathPrefix(target.data.id);
    }
  });
  cell.on("mouseover", (e,target) => showNodeText(target));

  // Append a tooltip.
  cell.append("title")
      //.text(d => `${d.ancestors().reverse().map(d => d.data.id).join(".")}\n${format(d.value)}`);
      .text(d => {Model.node_name_displayer(d.id,1); `${d.data.name}\n${format(d.value)}`;});

  cell.append("path")
      .attr("id", d => (d.cellUid = uid("cell")))
      .attr("fill", d => color(d.data.id))
      .attr("fill-opacity", 0.9)
      .attr("stroke", 'white')
      .attr("d", d => (`M ${d.x0} ${d.y0} L ${d.x1} ${d.y0} L ${d.x1} ${d.y1} L ${d.x0} ${d.y1} Z`));

  // Append a clipPath to ensure text does not overflow.
  cell.append("clipPath")
      .attr("id", d => (d.clipUid = uid("clip")))
    .append("use")
      .attr("xlink:href", d => d.cellUid.href);

  Model.treemap_svg_node = svg.node();
  setTreemapDetailLevel();

  container.replaceChildren(svg.node());
}

// Circular packmap generation, for visualizing tags.
function makePackmap(data) {

  const pack = d3.pack()
    .size([width, height])
    .padding(1);

  const root = pack(data);

  const svg = d3.create("svg")
      .attr("viewBox", [0, 0, width, height])
      .attr("width", width)
      .attr("height", height)
      .attr("style", `min-width: 25%;
                      max-width: 100%;
                      height: auto;
                      font: 15px sans-serif;
                      border: 1px solid black`)
      .attr("text-anchor", "middle");

  const node = svg.append("g")
      .selectAll()
      .data(root.descendants())
      .join("g")
        .attr("transform", d => `translate(${d.x},${d.y})`);

  node.append("circle")
      .attr("r", d => d.r ??  1)
      .attr("fill", d => d.children ? "white" : "#ccc")
      .attr("stroke", d => d.children ? "black" : null);

  const text = node
      .filter(d => !d.children && d.r > 20)
      .append("text")
          .attr("clip-path", d => `circle(${d.r})`);

  text.selectAll()
      .data(d => [d.data.id])
      .join("tspan")
          .text(d=>d);

  function showTagText(d) {
    Model.tag_name_displayer(d.data.id??'',1);
    Model.tag_name_displayer(`${d.value}`,2);
  }

  node.on("click", (e,target) => {
    if (e.shiftKey || e.ctrlKey || e.altKey || e.metaKey) {
      handleSetFocusTagOut();
    } else {
      handleSetFocusTag(target.data.id);
    }
  });
  node.on("mouseover", (e,target) => showTagText(target));

  tag_container.replaceChildren(svg.node());
}

function shouldBeVisible(d) {
  if (d.depth == Model.max_levels) { return true; }
  return d.depth < Model.max_levels && (!('children' in d));
}

function setTreemapDetailLevel() {
  if (Model.treemap_svg_node) {
    d3.select(Model.treemap_svg_node)
      .selectAll("g")
      .attr("visibility", (d) => (shouldBeVisible(d) ? "visible" : "hidden"));
  }
}

// File reading
function readFileToArrayBrowser(file) {
  return new Promise((resolve, reject) => {
    const reader = new FileReader();

    reader.onload = function(event) {
      const text = event.target.result;
      const lines = text.split('\n');
      resolve(lines);
    };

    reader.onerror = function(error) {
      reject(error);
    };

    reader.readAsText(file);
  });
}

async function readfile(file) {
  try {
    const lines = await readFileToArrayBrowser(file);
    return lines;
  } catch (error) {
    console.error('Error reading file:', error);
  }
  return [];
}

async function updateFromFile() {
  if (Model.file !== undefined) {
    Model.file_lines = await readfile(Model.file);

    resetFileDerivedData();
    updateFromLines();
  }
}

async function updateFromLines() {
  if (Model.file_lines === undefined) {
    return;
  }
  const rowPredicate = (row) => {
    if (row.pathString.indexOf(Model.path_substring) < 0) { return false; }
    if (!row.pathString.startsWith(Model.path_prefix)) { return false; }
    if (!Model.show_skip && row.verdict === 'Skip') { return false; }
    if (!Model.show_fail && row.verdict === 'Failure') { return false; }
    if (!Model.show_retry && row.verdict === 'RetryOnFailure') { return false; }
    if (!Model.show_slow && row.verdict === 'Slow') { return false; }
    const has_bug = ! row.bug.match(/\/0+$/);
    if (!Model.show_with_bug && has_bug) { return false; }
    if (!Model.show_without_bug && !has_bug) { return false; }
    // Reject if there is a focus tag that is not in the row.
    if (Model.focus_tags.filter(t => row.tags.indexOf(t) < 0).length > 0) { return false; }
    return true;
  };
  const rows = prestratifyExpectationLines(Model.file_lines, rowPredicate);
  const stratified = d3.stratify()
    .id((d) => d.id)
    .parentId((d) => d.parentId)
    (rows);

  Model.counted_data = d3.hierarchy(stratified).count();

  Model.treemap_id_to_node = {};
  Model.counted_data.descendants().forEach((d) => {Model.treemap_id_to_node[d.data.id] = d;});

  // Count occurrences of tags in filtered data.
  const tag_count = {};
  for (d of Model.counted_data.leaves()) {
    for (tag of (d.data.data.tags ?? [])) {
      tag_count[tag] = 1 + (tag_count[tag]??0);
    }
  }
  // Create parent-child rows for tag-group relations.
  const group_rows = [];
  // Link each tag to its tag group.
  const tag_group_dict = parseTagGroups(Model.file_lines);
  const live_groups = new Set();
  Model.live_tags = new Set();
  for (const [group, tags] of Object.entries(tag_group_dict)) {
    for (const tag of tags) {
      const entry = {id: tag, parentId: group, count: (tag_count[tag] ?? 0)};
      if (entry.count > 0) {
        group_rows.push(entry);
        live_groups.add(group);
        Model.live_tags.add(tag);
      }
    }
  }
  // Ensure there is a synthetic root.
  const kSyntheticTagRoot = 'All tags';
  group_rows.push({ id: kSyntheticTagRoot, count: 0 });
  // Link each tag groups to the synthetic root.
  for (const group of live_groups.values()) {
    group_rows.push({ id: group, parentId: kSyntheticTagRoot, count: 0 });
  }

  const stratified_tags = d3.stratify()
    .id(d => d.id)
    .parentId(d => d.parentId)
    (group_rows);

  const root = d3.hierarchy(stratified_tags);
  Model.counted_tags = root.sum(d => d.data.count).sort((a,b) => b.value - a.value);

  updateGraphs();

  // Display the rows, but only if they came from original text.
  // That is, don't display the synthetically created interior nodes.
  Model.numbered_lines_displayer(rows.filter((row) => row.hasOwnProperty('lineNumber')));
}

async function updateGraphs() {
  if (Model.counted_data !== undefined) {
    makeTreemap(Model.counted_data);
  }
  if (Model.counted_tags !== undefined) {
    makePackmap(Model.counted_tags);
  }
}


// Controller

async function handleInputFile(f) {
  Model.file = f;
  updateFromFile();
}

async function handleSetLevel(value) {
  Model.max_levels = value;
  setTreemapDetailLevel();
}

async function handleSetPathPrefix(path) {
  if (path === Model.path_prefix) {
    // The user clicked on the treemap node, but it's already
    // the whole treemap.  Increment level of detail if the
    // node has children.
    const node = (Model.treemap_id_to_node ?? {})[path] ?? {};
    if ((node.children ?? []).length > 0) {
      Model.detail_incrementer();
    }
  } else {
    Model.path_prefix = path;
    Model.node_name_displayer(Model.path_prefix, 0);
    updateFromLines();
  }
}

async function handleSetPathPrefixOut() {
  Model.path_prefix = parentOf(Model.path_prefix); // parentOf is in the other module.
  Model.node_name_displayer(Model.path_prefix, 0);
  updateFromLines();
}

async function handleSetPathSubstring(path) {
  if (path !== Model.path_substring) {
    console.log(`setting path substring ${path}`);
    Model.path_substring = path;
    updateFromLines();
  }
}

async function handleSetFocusTag(tag) {
  if (Model.live_tags.has(tag) && Model.focus_tags.indexOf(tag) < 0) {
    Model.focus_tags.push(tag);
    Model.tag_name_displayer(Model.focus_tags.join(' '), 0);
    updateFromLines();
  }
}

async function handleSetFocusTagOut() {
  if (Model.focus_tags.length > 0) {
    Model.focus_tags.pop();
    Model.tag_name_displayer(Model.focus_tags.join(' '), 0);
    updateFromLines();
  }
}

async function handleSetShow(name, checked) {
  Model[`show_${name}`] = checked;
  updateFromLines();
}

function SetNodeNameDisplayer(fn) {
  Model.node_name_displayer = fn;
}

function SetTagNameDisplayer(fn) {
  Model.tag_name_displayer = fn;
}

function SetNumberedLinesDisplayer(fn) {
  Model.numbered_lines_displayer = fn;
}

function SetDetailIncrementer(fn) {
  Model.detail_incrementer = fn;
}
