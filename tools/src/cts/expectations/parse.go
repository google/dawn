// Copyright 2022 The Dawn Authors
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

package expectations

import (
	"fmt"
	"strings"

	"dawn.googlesource.com/dawn/tools/src/cts/result"
)

const (
	tagHeaderStart = `BEGIN TAG HEADER`
	tagHeaderEnd   = `END TAG HEADER`
)

// Parse parses an expectations file, returning the Content
func Parse(path, body string) (Content, error) {
	// Normalize CRLF -> LF
	body = strings.ReplaceAll(body, "\r\n", "\n")

	// LineType is an enumerator classifying the 'type' of the line.
	type LineType int
	const (
		comment     LineType = iota // The line starts with the '#'
		expectation                 // The line declares an expectation
		blank                       // The line is blank
	)

	// classifyLine returns the LineType for the given line
	classifyLine := func(line string) LineType {
		line = strings.TrimSpace(line)
		switch {
		case line == "":
			return blank
		case strings.HasPrefix(line, "#"):
			return comment
		default:
			return expectation
		}
	}

	content := Content{} // The output content

	var pending Chunk // The current Chunk being parsed

	// flush completes the current chunk, appending it to 'content'
	flush := func() {
		parseTags(&content.Tags, pending.Comments)
		content.Chunks = append(content.Chunks, pending)
		pending = Chunk{}
	}

	lastLineType := blank                         // The type of the last parsed line
	for i, l := range strings.Split(body, "\n") { // For each line...
		lineIdx := i + 1 // line index
		lineType := classifyLine(l)

		// Compare the new line type to the last.
		// Flush the pending chunk if needed.
		if i > 0 {
			switch {
			case
				lastLineType == blank && lineType != blank,             // blank -> !blank
				lastLineType != blank && lineType == blank,             // !blank -> blank
				lastLineType == expectation && lineType != expectation: // expectation -> comment
				flush()
			}
		}

		lastLineType = lineType

		// Handle blank lines and comments.
		switch lineType {
		case blank:
			continue
		case comment:
			pending.Comments = append(pending.Comments, l)
			continue
		}

		// Below this point, we're dealing with an expectation

		// Split the line by whitespace to form a list of tokens
		type Token struct {
			str        string
			start, end int // line offsets (0-based)
		}
		tokens := []Token{}
		if len(l) > 0 { // Parse the tokens
			inToken, s := false, 0
			for i, c := range l {
				if c == ' ' {
					if inToken {
						tokens = append(tokens, Token{l[s:i], s, i})
						inToken = false
					}
				} else if !inToken {
					s = i
					inToken = true
				}
			}
			if inToken {
				tokens = append(tokens, Token{l[s:], s, len(l)})
			}
		}

		// syntaxErr is a helper for returning a SyntaxError with the current
		// line and column index.
		syntaxErr := func(at Token, msg string) error {
			columnIdx := at.start + 1
			if columnIdx == 1 {
				columnIdx = len(l) + 1
			}
			return fmt.Errorf("%v:%v:%v error: %v", path, lineIdx, columnIdx, msg)
		}

		// peek returns the next token without consuming it.
		// If there are no more tokens then an empty Token is returned.
		peek := func() Token {
			if len(tokens) > 0 {
				return tokens[0]
			}
			return Token{}
		}

		// next returns the next token, consuming it and incrementing the
		// column index.
		// If there are no more tokens then an empty Token is returned.
		next := func() Token {
			if len(tokens) > 0 {
				tok := tokens[0]
				tokens = tokens[1:]
				return tok
			}
			return Token{}
		}

		match := func(str string) bool {
			if peek().str != str {
				return false
			}
			next()
			return true
		}

		// tags parses a [ tag ] block.
		tags := func(use string) (result.Tags, error) {
			if !match("[") {
				return result.Tags{}, nil
			}
			out := result.NewTags()
			for {
				t := next()
				switch t.str {
				case "]":
					return out, nil
				case "":
					return result.Tags{}, syntaxErr(t, "expected ']' for "+use)
				default:
					out.Add(t.str)
				}
			}
		}

		// Parse the optional bug
		var bug string
		if strings.HasPrefix(peek().str, "crbug.com") {
			bug = next().str
		}

		// Parse the optional test tags
		testTags, err := tags("tags")
		if err != nil {
			return Content{}, err
		}

		// Parse the query
		if t := peek(); t.str == "" || t.str[0] == '#' || t.str[0] == '[' {
			return Content{}, syntaxErr(t, "expected test query")
		}
		query := next().str

		// Parse the expected status
		if t := peek(); !strings.HasPrefix(t.str, "[") {
			return Content{}, syntaxErr(t, "expected status")
		}
		status, err := tags("status")
		if err != nil {
			return Content{}, err
		}

		// Parse any optional trailing comment
		comment := ""
		if t := peek(); strings.HasPrefix(t.str, "#") {
			comment = l[t.start:]
		}

		// Append the expectation to the list.
		pending.Expectations = append(pending.Expectations, Expectation{
			Line:    lineIdx,
			Bug:     bug,
			Tags:    testTags,
			Query:   query,
			Status:  status.List(),
			Comment: comment,
		})
	}

	if lastLineType != blank {
		flush()
	}

	return content, nil
}

// parseTags parses the tag information found between tagHeaderStart and
// tagHeaderEnd comments.
func parseTags(tags *Tags, lines []string) {
	// Flags for whether we're currently parsing a TAG HEADER and whether we're
	// also within a tag-set.
	inTagsHeader, inTagSet := false, false
	tagSet := TagSet{} // The currently parsed tag-set
	for _, line := range lines {
		line = strings.TrimSpace(strings.TrimLeft(strings.TrimSpace(line), "#"))
		if strings.Contains(line, tagHeaderStart) {
			if tags.ByName == nil {
				*tags = Tags{
					ByName: map[string]TagSetAndPriority{},
					Sets:   []TagSet{},
				}
			}
			inTagsHeader = true
			continue
		}
		if strings.Contains(line, tagHeaderEnd) {
			return // Reached the end of the TAG HEADER
		}
		if !inTagsHeader {
			continue // Still looking for a tagHeaderStart
		}

		// Below this point, we're in a TAG HEADER.
		tokens := removeEmpty(strings.Split(line, " "))
		for len(tokens) > 0 {
			if inTagSet {
				// Parsing tags in a tag-set (between the '[' and ']')
				if tokens[0] == "]" {
					// End of the tag-set.
					tags.Sets = append(tags.Sets, tagSet)
					inTagSet = false
					break
				} else {
					// Still inside the tag-set. Consume the tag.
					tag := tokens[0]
					tags.ByName[tag] = TagSetAndPriority{
						Set:      tagSet.Name,
						Priority: len(tagSet.Tags),
					}
					tagSet.Tags.Add(tag)
				}
				tokens = tokens[1:]
			} else {
				// Outside of tag-set. Scan for 'tags: ['
				if len(tokens) > 2 && tokens[0] == "tags:" && tokens[1] == "[" {
					inTagSet = true
					tagSet.Tags = result.NewTags()
					tokens = tokens[2:] // Skip 'tags:' and '['
				} else {
					// Tag set names are on their own line.
					// Remember the content of the line, in case the next line
					// starts a tag-set.
					tagSet.Name = strings.Join(tokens, " ")
					break
				}
			}
		}
	}
}

// removeEmpty returns the list of strings with all empty strings removed.
func removeEmpty(in []string) []string {
	out := make([]string, 0, len(in))
	for _, s := range in {
		if s != "" {
			out = append(out, s)
		}
	}
	return out
}
