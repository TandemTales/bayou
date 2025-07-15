# AGENTS.md - AI Agent Guidelines for Bayou Bonanza

This document provides comprehensive guidelines for AI agents working on the Bayou Bonanza project. It consolidates all rules from the `.cursor/rules` folder into a single reference document.

## Table of Contents

1. [Project Overview](#project-overview)
2. [C++ Organization Rules](#c-organization-rules)
3. [Task Master Development Workflow](#task-master-development-workflow)
4. [Task Master Tools & Commands](#task-master-tools--commands)
5. [Rule Management & Self-Improvement](#rule-management--self-improvement)
6. [Development Best Practices](#development-best-practices)

---

## Project Overview

Bayou Bonanza is a C++ game project using SFML for graphics and networking. The project follows a structured approach with:

- **Header files**: Located in `include/` directory
- **Source files**: Located in `src/` directory  
- **Tests**: Located in `tests/` directory
- **Task Management**: Using Task Master for project organization
- **Build System**: CMake with proper include path configuration

---

## C++ Organization Rules

### **Header File Placement**
- **ALL header files (.h) MUST be placed in the `include/` directory**
- **NEVER place header files in the `src/` directory**
- Source files (.cpp) belong in the `src/` directory
- Test files (.cpp) belong in the `tests/` directory

```cpp
// ✅ DO: Correct file structure
include/
├── GameBoard.h
├── Piece.h
├── CombatSystem.h
└── PlayerSide.h

src/
├── GameBoard.cpp
├── Piece.cpp
├── CombatSystem.cpp
└── main.cpp

// ❌ DON'T: Header files in src directory
src/
├── GameBoard.h     // WRONG - should be in include/
├── GameBoard.cpp
├── Piece.h         // WRONG - should be in include/
└── Piece.cpp
```

### **Include Path Guidelines**
- **Use relative paths from src/ to include/ when necessary**
- **Prefer using the build system's include path (-I./include)**
- **Use quotes for project headers, angle brackets for system headers**

```cpp
// ✅ DO: Correct include statements
#include "GameBoard.h"           // When build system includes ./include
#include "../include/GameBoard.h" // When explicit path needed
#include <SFML/Network.hpp>      // System/library headers

// ❌ DON'T: Incorrect include patterns
#include <GameBoard.h>           // Wrong - use quotes for project headers
#include "SFML/Network.hpp"      // Wrong - use angle brackets for system headers
```

### **File Naming Conventions**
- **Use PascalCase for class header/source files**
- **Match header and source file names exactly**
- **Use descriptive, clear names that reflect the class/module purpose**

### **Header Guard Standards**
- **Use include guards in ALL header files**
- **Follow the pattern: `BAYOU_BONANZA_CLASSNAME_H`**
- **Use uppercase with underscores**

```cpp
// ✅ DO: Proper header guards
#ifndef BAYOU_BONANZA_COMBAT_SYSTEM_H
#define BAYOU_BONANZA_COMBAT_SYSTEM_H

// Header content here

#endif // BAYOU_BONANZA_COMBAT_SYSTEM_H
```

### **Namespace Organization**
- **All project code MUST be in the `BayouBonanza` namespace**
- **Use namespace consistently across all files**
- **Close namespace with comment indicating the namespace name**

```cpp
// ✅ DO: Proper namespace usage
namespace BayouBonanza {

class CombatSystem {
    // Class implementation
};

} // namespace BayouBonanza
```

### **Forward Declarations**
- **Use forward declarations in headers when possible**
- **Include full headers only when necessary (inheritance, composition)**
- **Prefer forward declarations to reduce compilation dependencies**

---

## Task Master Development Workflow

### Primary Interaction Methods

**MCP Server (Recommended for AI Agents)**:
- For AI agents and integrated development environments (like Cursor), use the **MCP server**
- Better performance, structured data exchange, and richer error handling
- Exposes Task Master functionality through tools (e.g., `get_tasks`, `add_subtask`)

**CLI (Fallback)**:
- Global `task-master` command for direct terminal interaction
- Serves as fallback if MCP server is inaccessible
- Commands mirror MCP tools (e.g., `task-master list` corresponds to `get_tasks`)

### Standard Development Process

1. **Project Initialization**:
   - Run `initialize_project` tool / `task-master init`
   - Or use `parse_prd` / `task-master parse-prd --input='<prd-file.txt>'` to generate initial tasks.json

2. **Session Start**:
   - Begin with `get_tasks` / `task-master list` to see current tasks, status, and IDs
   - Use `next_task` / `task-master next` to determine the next task to work on

3. **Task Analysis**:
   - Run `analyze_project_complexity` / `task-master analyze-complexity --research` before breaking down tasks
   - Review with `complexity_report` / `task-master complexity-report`

4. **Task Breakdown**:
   - Use `expand_task` / `task-master expand --id=<id> --force --research` for complex tasks
   - Clear existing subtasks with `clear_subtasks` / `task-master clear-subtasks --id=<id>` if needed

5. **Implementation**:
   - View task details with `get_task` / `task-master show <id>`
   - Follow iterative subtask implementation process (detailed below)
   - Mark completed tasks with `set_task_status` / `task-master set-status --id=<id> --status=done`

6. **Maintenance**:
   - Update tasks when implementation differs using `update` or `update_task`
   - Add new tasks with `add_task` / `task-master add-task --prompt="..." --research`
   - Generate task files with `generate` / `task-master generate`

### Iterative Subtask Implementation

1. **Understand the Goal**: Use `get_task` to understand subtask requirements
2. **Initial Planning**: Explore codebase, identify files/functions to modify
3. **Log the Plan**: Use `update_subtask` to record detailed implementation plan
4. **Verify the Plan**: Confirm plan was logged successfully
5. **Begin Implementation**: Set status to 'in-progress' and start coding
6. **Refine and Log Progress**: Regularly update subtask with findings:
   - What worked ("fundamental truths" discovered)
   - What didn't work and why
   - Specific code snippets or configurations that were successful
   - Decisions made and reasoning
7. **Review & Update Rules**: Update project rules based on new patterns
8. **Mark Complete**: Set status to 'done' after verification
9. **Commit Changes**: Stage changes and create comprehensive commit message
10. **Proceed**: Move to next subtask

### Task Structure Fields

- **id**: Unique identifier (e.g., `1`, `1.1`)
- **title**: Brief, descriptive title
- **description**: Concise summary of the task
- **status**: Current state (`pending`, `done`, `deferred`, etc.)
- **dependencies**: IDs of prerequisite tasks
- **priority**: Importance level (`high`, `medium`, `low`)
- **details**: In-depth implementation instructions
- **testStrategy**: Verification approach
- **subtasks**: List of smaller, more specific tasks

### Configuration Management

**`.taskmasterconfig` File (Primary)**:
- Located in project root
- Stores AI model selections, parameters, logging level, etc.
- Managed via `task-master models --setup` command
- **Do not edit manually**

**Environment Variables**:
- Used only for sensitive API keys and endpoint URLs
- Place in `.env` file for CLI usage
- Configure in `.cursor/mcp.json` env section for MCP/Cursor integration

---

## Task Master Tools & Commands

### Initialization & Setup

**Initialize Project**:
- **MCP Tool**: `initialize_project`
- **CLI**: `task-master init [options]`
- **Purpose**: Set up basic Taskmaster file structure and configuration

**Parse PRD**:
- **MCP Tool**: `parse_prd`
- **CLI**: `task-master parse-prd [file] [options]`
- **Purpose**: Parse Product Requirements Document to generate initial tasks
- **Note**: Makes AI calls, can take up to a minute

### AI Model Configuration

**Manage Models**:
- **MCP Tool**: `models`
- **CLI**: `task-master models [options]`
- **Purpose**: View/set AI model configuration for different roles
- **Key Parameters**: `setMain`, `setResearch`, `setFallback`, `listAvailableModels`

### Task Listing & Viewing

**Get Tasks**:
- **MCP Tool**: `get_tasks`
- **CLI**: `task-master list [options]`
- **Purpose**: List tasks with optional filtering by status

**Get Next Task**:
- **MCP Tool**: `next_task`
- **CLI**: `task-master next [options]`
- **Purpose**: Show next available task based on dependencies

**Get Task Details**:
- **MCP Tool**: `get_task`
- **CLI**: `task-master show [id] [options]`
- **Purpose**: Display detailed information for specific task/subtask

### Task Creation & Modification

**Add Task**:
- **MCP Tool**: `add_task`
- **CLI**: `task-master add-task [options]`
- **Purpose**: Add new task using AI description
- **Note**: Makes AI calls, can take up to a minute

**Add Subtask**:
- **MCP Tool**: `add_subtask`
- **CLI**: `task-master add-subtask [options]`
- **Purpose**: Add subtask to parent task or convert existing task

**Update Tasks**:
- **MCP Tool**: `update`
- **CLI**: `task-master update [options]`
- **Purpose**: Update multiple upcoming tasks based on new context
- **Note**: Makes AI calls, can take up to a minute

**Update Task**:
- **MCP Tool**: `update_task`
- **CLI**: `task-master update-task [options]`
- **Purpose**: Modify specific task with new information
- **Note**: Makes AI calls, can take up to a minute

**Update Subtask**:
- **MCP Tool**: `update_subtask`
- **CLI**: `task-master update-subtask [options]`
- **Purpose**: Append timestamped notes to subtask for iterative logging
- **Note**: Makes AI calls, can take up to a minute

**Set Task Status**:
- **MCP Tool**: `set_task_status`
- **CLI**: `task-master set-status [options]`
- **Purpose**: Update task/subtask status

**Remove Task**:
- **MCP Tool**: `remove_task`
- **CLI**: `task-master remove-task [options]`
- **Purpose**: Permanently remove task/subtask

### Task Structure & Breakdown

**Expand Task**:
- **MCP Tool**: `expand_task`
- **CLI**: `task-master expand [options]`
- **Purpose**: Break down complex task into subtasks using AI
- **Note**: Makes AI calls, can take up to a minute

**Expand All Tasks**:
- **MCP Tool**: `expand_all`
- **CLI**: `task-master expand --all [options]`
- **Purpose**: Expand all eligible tasks based on complexity analysis
- **Note**: Makes AI calls, can take up to a minute

**Clear Subtasks**:
- **MCP Tool**: `clear_subtasks`
- **CLI**: `task-master clear-subtasks [options]`
- **Purpose**: Remove all subtasks from parent tasks

### Dependency Management

**Add Dependency**:
- **MCP Tool**: `add_dependency`
- **CLI**: `task-master add-dependency [options]`
- **Purpose**: Define task prerequisite relationships

**Remove Dependency**:
- **MCP Tool**: `remove_dependency`
- **CLI**: `task-master remove-dependency [options]`
- **Purpose**: Remove dependency relationship

**Validate Dependencies**:
- **MCP Tool**: `validate_dependencies`
- **CLI**: `task-master validate-dependencies [options]`
- **Purpose**: Check for dependency issues without making changes

**Fix Dependencies**:
- **MCP Tool**: `fix_dependencies`
- **CLI**: `task-master fix-dependencies [options]`
- **Purpose**: Automatically fix dependency issues

### Analysis & Reporting

**Analyze Project Complexity**:
- **MCP Tool**: `analyze_project_complexity`
- **CLI**: `task-master analyze-complexity [options]`
- **Purpose**: Analyze task complexity and suggest breakdown
- **Note**: Makes AI calls, can take up to a minute

**View Complexity Report**:
- **MCP Tool**: `complexity_report`
- **CLI**: `task-master complexity-report [options]`
- **Purpose**: Display complexity analysis in readable format

### File Management

**Generate Task Files**:
- **MCP Tool**: `generate`
- **CLI**: `task-master generate [options]`
- **Purpose**: Create/update individual Markdown files for each task

---

## Rule Management & Self-Improvement

### Rule Improvement Triggers

- New code patterns not covered by existing rules
- Repeated similar implementations across files
- Common error patterns that could be prevented
- New libraries or tools being used consistently
- Emerging best practices in the codebase

### Analysis Process

- Compare new code with existing rules
- Identify patterns that should be standardized
- Look for references to external documentation
- Check for consistent error handling patterns
- Monitor test patterns and coverage

### Rule Updates

**Add New Rules When**:
- A new technology/pattern is used in 3+ files
- Common bugs could be prevented by a rule
- Code reviews repeatedly mention the same feedback
- New security or performance patterns emerge

**Modify Existing Rules When**:
- Better examples exist in the codebase
- Additional edge cases are discovered
- Related rules have been updated
- Implementation details have changed

### Rule Quality Checks

- Rules should be actionable and specific
- Examples should come from actual code
- References should be up to date
- Patterns should be consistently enforced

### Continuous Improvement

- Monitor code review comments
- Track common development questions
- Update rules after major refactors
- Add links to relevant documentation
- Cross-reference related rules

---

## Development Best Practices

### Rule Structure Requirements

```markdown
---
description: Clear, one-line description of what the rule enforces
globs: path/to/files/*.ext, other/path/**/*
alwaysApply: boolean
---

- **Main Points in Bold**
  - Sub-points with details
  - Examples and explanations
```

### File References

- Use `[filename](mdc:path/to/file)` format for file references
- Example: `[schema.prisma](mdc:prisma/schema.prisma)` for code references

### Code Examples

```cpp
// ✅ DO: Show good examples
const goodExample = true;

// ❌ DON'T: Show anti-patterns
const badExample = false;
```

### Rule Content Guidelines

- Start with high-level overview
- Include specific, actionable requirements
- Show examples of correct implementation
- Reference existing code when possible
- Keep rules DRY by referencing other rules

### Best Practices

- Use bullet points for clarity
- Keep descriptions concise
- Include both DO and DON'T examples
- Reference actual code over theoretical examples
- Use consistent formatting across rules

---

## Environment Variables Configuration

### API Keys (Required for corresponding provider)

- `ANTHROPIC_API_KEY`
- `PERPLEXITY_API_KEY`
- `OPENAI_API_KEY`
- `GOOGLE_API_KEY`
- `MISTRAL_API_KEY`
- `AZURE_OPENAI_API_KEY` (Requires `AZURE_OPENAI_ENDPOINT`)
- `OPENROUTER_API_KEY`
- `XAI_API_KEY`
- `OLLAMA_API_KEY` (Requires `OLLAMA_BASE_URL`)

### Endpoints (Optional/Provider Specific)

- `AZURE_OPENAI_ENDPOINT`
- `OLLAMA_BASE_URL` (Default: `http://localhost:11434/api`)

### Configuration Notes

- Set API keys in `.env` file (CLI use) or `.cursor/mcp.json` env section (MCP/Cursor)
- All other settings managed in `.taskmasterconfig` via `task-master models` command
- **Never manually edit `.taskmasterconfig`**

---

## Summary

This document serves as a comprehensive guide for AI agents working on the Bayou Bonanza project. It covers:

1. **C++ Organization**: Strict file placement, naming, and structure rules
2. **Task Management**: Complete workflow using Task Master tools
3. **Development Process**: Iterative implementation with detailed logging
4. **Rule Management**: Continuous improvement and pattern recognition
5. **Tool Reference**: Complete MCP and CLI command documentation

Always prioritize MCP tools over CLI commands when available, follow the iterative subtask implementation process, and continuously update rules based on emerging patterns in the codebase. 