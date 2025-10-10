## 📚 Index

1. [Installation](#installation)
2. [Directories](#directories)
3. [modes](#modes)
4. [Commands](#commands)

- [init](#init)
- [cat-file](#cat-file)
- [hash-object](#hash-object)
- [add](#add)
- [write-tree](#write-tree)
- [ls-tree](#ls-tree)
- [commit](#commit)
- [status](#status)
- [log](#log)
- [branch](#branch)
- [diff](#diff)
- [checkout](#checkout)
- [merge](#merge)
- [reset](#reset)
- [stash](#stash)

---

# **Installation**

You must install the required packages:

```bash
sudo apt update
sudo apt install g++
sudo apt-get install libssl-dev
sudo apt install zlib1g-dev
```
| **Command**                       | **Library / Package** | **One-liner Description**                                                                 |
| --------------------------------- | --------------------- | ----------------------------------------------------------------------------------------- |
| `sudo apt update`                 | -                     | Updates the local package index to fetch the latest available package versions.           |
| `sudo apt install g++`            | `g++`                 | Installs the GNU C++ compiler for compiling C++ source code.                              |
| `sudo apt-get install libssl-dev` | `libssl-dev`          | Installs OpenSSL development libraries for implementing cryptographic functions like SHA. |
| `sudo apt install zlib1g-dev`     | `zlib1g-dev`          | Installs the zlib development files for compression and decompression functionality.      |


Once the dependencies are installed, you can build the project using:

```bash
make all
```
(make sure to create an empty folder named 'test' before running this command)

Use this command to create a symbolic link and access 'vcs' globally:

```bash
sudo ln -s /vcs/test/main.out /usr/local/bin/vcs
```
---

# **Directories**

```bash
kishan@my-pc:~/vcs$ tree
.
├── include
│   ├── command_executor.hpp
│   ├── command_parser.hpp
│   ├── commands
│   │   ├── add.hpp
│   │   ├── branch.hpp
│   │   ├── cat-file.hpp
│   │   ├── checkout.hpp
│   │   ├── commit.hpp
│   │   ├── diff.hpp
│   │   ├── hash-object.hpp
│   │   ├── init.hpp
│   │   ├── log.hpp
│   │   ├── ls-tree.hpp
│   │   ├── merge.hpp
│   │   ├── reset.hpp
│   │   ├── revert.hpp
│   │   ├── stash.hpp
│   │   ├── status.hpp
│   │   └── write-tree.hpp
│   ├── commands.hpp
│   ├── config.hpp
│   ├── exceptions
│   │   └── vcs-exception.hpp
│   ├── models
│   │   ├── index.hpp
│   │   └── tree.hpp
│   ├── utils.hpp
│   └── vcs.hpp
├── Makefile
├── README.md
├── src
│   ├── command_executor.cpp
│   ├── command_parser.cpp
│   ├── commands
│   │   ├── add.cpp
│   │   ├── branch.cpp
│   │   ├── cat-file.cpp
│   │   ├── checkout.cpp
│   │   ├── commit.cpp
│   │   ├── diff.cpp
│   │   ├── hash-object.cpp
│   │   ├── init.cpp
│   │   ├── log.cpp
│   │   ├── ls-tree.cpp
│   │   ├── merge.cpp
│   │   ├── reset.cpp
│   │   ├── revert.cpp
│   │   ├── stash.cpp
│   │   ├── status.cpp
│   │   └── write-tree.cpp
│   ├── commands.cpp
│   ├── exceptions
│   │   └── vcs-exception.cpp
│   ├── main.cpp
│   ├── models
│   │   └── tree.cpp
│   ├── utils.cpp
│   └── vcs.cpp
└── test
    └── main.out

10 directories, 52 files
```

---

# **modes**

```
120000  // Symbolic link
100755  // Executable file
100644  // Regular file
040000  // Directory (tree)
```

---

# **Commands**

# **`init`**

```bash
vcs init
```

- This command will create and initialize the `.vcs` directory.

- The file and folder structure looks like:

![img1](screenshots/init/init_1.png)

- `.vcs/index` is the staging area where all tracked files are tracked or recorded.
- `.vcs/refs/heads/` contains all branches in the form of files. Every new branch is added to this directory as a new file.
- Similarly, `.vcs/logs/refs/heads/` contains the commit logs for each specific branch.
- `HEAD` file initially contains `ref: refs/heads/master`, which means you are on the `master` branch. If you are in a detached `HEAD` state, the `HEAD` file contains a commit hash, meaning `HEAD` is pointing directly to a specific commit.
- `.vcs/refs/heads/master` file initially contains: `0000000000000000000000000000000000000000`
- `.vcs/refs/stash` file initially contains: `0000000000000000000000000000000000000000`
- `.vcsignore` contains all files and directories to be ignored by the `vcs`.

---

# **`Testing`**

- For testing, we have created a `test/` directory. So, for testing purposes, we will create a directory structure as shown below.

![img2](screenshots/init/init_2.png)

---

# **`cat-file`**

```bash
vcs cat-file <type> <object>
```

![img5](screenshots/cat-file/cat-file_1.png)

- Displays information about an object stored in the `.vcs/objects` directory.

---

### &#10140; **Arguments**

### **`<type>`**

- `-e` — Check if the specified object exists.
- `-p` — Pretty-print the content of the object.
- `-t` — Display the type of the object (`blob`, `tree`, or `commit`).

### **`<object>`**

- A 40-character `SHA-1` hash representing the object ID.

---

### &#10140; **How It Works**

The command uses the given object hash to locate the object inside `.vcs/objects`.

`.vcs/objects/obj-hash[1-2]/obj-hash[3-40]`

It splits the hash:

- The first 2 characters become the directory name.
- The remaining 38 characters become the file name inside that directory.
- If the object file is found, it gets decompressed and handled according to the selected option (`-p`, `-e`, or `-t`)."

---

### &#10140; **`blob-object` File Format:**

```text
blob <size>\0<content>
```

---

# **`hash-object`**

```bash
vcs hash-object [-w] <file-path>
```

![img3](screenshots/hash-object/hash-object_1.png)

![img4](screenshots/hash-object/hash-object_2.png)

- Generates `SHA-1` hash

### &#10140; **Arguments**

### **`-w`** (optional)

* Generates `SHA-1` hash and write to `.vcs/objects/` directory

### **`<file-path>`**

* Uses the given file path to store the object inside the `.vcs/objects/` directory.

---

### &#10140; **How It Works**

- The content of the provided file path is first used to generate a `SHA-1` hash.

- If the `-w` flag is provided, the file content is compressed and stored as an object in `.vcs/objects/obj-hash[1-2]/obj-hash[3-40]`.

---

# **`add`**

```bash
vcs add [-s] <file-path>
```

![img6](screenshots/add/add_1.png)

![img7](screenshots/add/add_2.png)

- Recursively adds all files inside the given `file-path` to the staging area.
- All added files are now tracked through `.vcs/index` file.

### &#10140; **Arguments**

### **`-s`** (optional)

- Print status of the all added file with `SHA-1` hash and `file-path`

### **`<file-path>`**

- The path to the file or directory containing files to be added to the staging area.

---

### &#10140; **How It Works**

- First, recursively traverse all files and directories inside the given `file-path`.

- For each file found:

	- Convert it into a **blob object**.
	- Store the object in the `.vcs/objects/` directory.
	- Add an entry for the file in the `index` file (staging area) located at `.vcs/index`.
	- The `.vcs/index` is compressed after all entries have been added.

---

### &#10140; **`index` File Format:**

Each line in the `index` file represents a tracked file in the following format:

```text
<relative-file-path> <sha1-hash> <file-size> <mode> <mtime>
```

#### Example:

```text
test3/cc.txt f9f44ed2913cdec196c03250aa094e7808a81cd1 20 100644 1751267531
test2/b.txt c81194658ef9daf800c5fc209fb6818168f7bfc7 19 100644 1751267441
test2/bb.txt 25d25c1aac0f46d3fe7d03743a46b457cd794215 20 100644 1751267467
test1/aa.txt 56a7123021727995b430b88385992fb332a1c793 20 100644 1751267416
test3/c.txt 08029ce0e971fbc04f9e9599d79306ff6fac31d1 19 100644 1751267502
test1/a.txt 39a4f5e6c2aab4f667933035585ef4a6402f5e17 20 100644 1751267397
demo.txt d5aceb90ab934b1dce7162b24a7b7bdae3d66ff3 22 100644 1751267342
.vcsignore 55eb8f7ed034702f901fa78be60f9a5a1d1f16fd 8 100644 1751270039
```

- `mtime` stands for modification time — the last time the file was modified.
- If a file has an entry in the `index`, it is considered **tracked** by the version control system (vcs).

---

# **`ls-tree`**

```bash
vcs ls-tree <object>
```

![img8](screenshots/ls-tree/ls-tree_1.png)


- Print the `tree-object` 

### &#10140; **Arguments**

### **`<object>`**

- `SHA-1` hash representing the `tree-object`.

---

### &#10140; **How It Works**

- First, it checks whether the object is a valid `tree-object`.
- If it is a valid `tree-object`, it searches in the `.vcs/objects/` directory, and prints the tree.

---

### &#10140; **`tree-object` File Format:**

```text
tree <size>\0
<mode> <type> <hash> <mtim> <size> <relative-file-path>
<mode> <type> <hash> <mtim> <size> <relative-file-path>
```

---

# **`write-tree`**

```bash
vcs write-tree [-s]
```

![img9](screenshots/write-tree/write-tree_1.png)

- Traverses the `.vcs/index` (staging area) file, and at each directory level, generates a `tree-object` and stores it in `.vcs/objects/`.

### &#10140; **Arguments**

### **`-s`** (optional)

- Print status of the all generated `tree-object` with `SHA-1`.

---

### &#10140; **How It Works**

- First, traverse the `.vcs/index` (staging area) file and store the data of each staged file in the `IndexEntry` `struct`.

#### **`include/models/index.hpp`**

```cpp
struct IndexEntry {
    std::string filepath;
    std::string hash;
    std::string size;
    std::string mode;
    std::time_t mtime;

    IndexEntry() {}

    IndexEntry(std::string filepath, std::string hash, std::string size, std::string mode, std::time_t mtime) : filepath(filepath), hash(hash), size(size), mode(mode), mtime(mtime) {}
};
```

- For each `IndexEntry`, insert the file path into the Tree class to generate a tree-like structure with parent and child nodes.

- Each non-leaf node represents a directory.
- Each leaf node represents a file.

#### **`include/models/tree.hpp`**

```cpp
class Node {
public:
    std::string fs_name;
    std::map<std::string, Node*> children; // {children_name, address}, {children_name, address}
    IndexEntry index_entry;

    Node(const std::string name) : fs_name(name){}
};

class Tree {
private:
    void delete_nodes(Node* node);

public:
    Node* root;

    Tree() : root(new Node(".")) {}

    ~Tree() { delete_nodes(root); }

    void insert(const IndexEntry& index_entry);  
};
```

- Now, recursively traverse the tree. During backtracking, create a `tree-object` at each directory level.

- After completing the traversal, you will end up at `"."` (the root), and obtain the root `tree-object`.

---

# **`commit`**

```bash
vcs commit <message>
```

![img10](screenshots/commit/commit_1.png)

![img11](screenshots/commit/commit_2.png)

- If anything has changed with respect to the previous commit, a `commit-object` is created and stored in the `.vcs/objects/` directory.

### &#10140; **Arguments**

### **`<message>`**

- A short, clear summary describing the purpose of the commit.

---

### &#10140; **How It Works**

- First, it internally executes the `write-tree` command to obtain the `tree-hash`. Using this, it creates a `commit-object` and generates a `commit-hash`.

- Retrieves the current branch (cur-branch) from `.vcs/HEAD`.

- If `.vcs/HEAD` contains a commit hash but not `ref: refs/heads/<cur-branch>`, committing is not allowed because `HEAD` is in a detached state.

- Initially, `.vcs/refs/heads/<cur-branch>` contains the last commit of the `current branch`, which becomes the parent of the new commit.

- The head of the current branch is then updated by writing the new commit_hash to `.vcs/refs/heads/<cur-branch>`.

- Finally, the current commit is logged by appending an entry to `.vcs/logs/refs/<cur-branch>`.

### &#10140; **`commit-object` File Format:**

```text
commit <size>\0
tree <tree-hash>
parent <parent-commit-hash>
author <username> <timestamp>
committer <username> <timestamp>
<message>
```

#### **Example:**

```
tree e5eb34217e6417017df52ff848670e2f53dc6f47
parent 0000000000000000000000000000000000000000
author kishan 1751277581
committer kishan 1751277581
first commit
```

---

# **`status`**

```bash
vcs status
```

- Shows the status of the staging area vs. the current working directory, and the last commit vs. the staging area.

- Now, I changed the content of `demo.txt`, deleted `test3/cc.txt`, created a new file `dummy_file.txt`, and created and added `file1.txt` to the staging area.

![img12](screenshots/status/status_1.png)

![img13](screenshots/status/status_2.png)

---

### &#10140; **How It Works**

- It first compares the current working directory with the staging area. If any modified, deleted, or new files are found, they are printed in red, indicating that they are not in the staging area and therefore not ready to be committed.

- Then, it compares the staging area with the last commit. If any modified, deleted, or new files are found, they are printed in green, indicating that they are staged and ready to be committed.

---

# **`log`**

```bash
vcs log
```

- Shows the commit logs of the current branch, starting from the first commit up to the latest.

![img14](screenshots/log/log_1.png)

---

### &#10140; **How It Works**

- Fetches the `HEAD` commit hash.
- Iterates through all branch logs in `.vcs/logs/refs/heads/<branch>` and records(stores) all parent commits of each commit.
- Then backtracks from the `HEAD` commit hash to the root commit hash `0000000000000000000000000000000000000000` and prints the path.

### &#10140; **`.vcs/logs/refs/heads/<branch>` File Format:**

```text
<parent-hash> <commit-hash> <username> <timestamp> commit: <message>
```

#### **Example:**

```text
0000000000000000000000000000000000000000 506d22d7854672a9c19793d654df4f90d619937c kishan 1760093545 commit: First Commit!
506d22d7854672a9c19793d654df4f90d619937c 2499f28a695fa31628c31bf98b448ce9ec034ef5 kishan 1760094289 commit: Second Commit
```
---

# **`branch`**

```bash
vcs branch
```

- This will print all branches

```bash
vcs branch <new-branch-name>
```

- This will create a new branch at the current `HEAD` commit hash.

![img15](screenshots/branch/branch_1.png)

```bash
vcs branch <new-branch-name> <source-hash>
```

- This will create a new branch at `source-hash`.

![img16](screenshots/branch/branch_2.png)

```bash
vcs branch <new-branch-name> <source-branch-name>
```

- This will create a new branch at `HEAD` of `source-branch`.

![img17](screenshots/branch/branch_3.png)


---

### &#10140; **How It Works**

- This will create a new file named `new-branch-name` in the `.vcs/refs/heads/` directory and store the provided `source-hash` or the `HEAD` of the specified `source-branch` in it.

---

# **`diff`**

```bash
vcs diff
```

- This will print diff between Staging Area to Working Directory.

![img18](screenshots/diff/diff_1.png)

```bash
vcs diff --staged
vcs diff --cached
```

- This will print diff between Staging Area vs Last Commit.

![img19](screenshots/diff/diff_2.png)

```bash
vcs diff <branch1> <branch2>
```

- This will print diff Branch1 vs Branch2.

![img20](screenshots/diff/diff_3.png)

```bash
vcs diff <commit1> <commit1>
```

- This will print diff Commit1 vs Commit2

![img21](screenshots/diff/diff_4.png)

---

### &#10140; **How It Works**

- First, it retrieves `HEAD1` and `HEAD2` and compares them. If a file is created, deleted, or its mode is changed, it prints the differences. If a file is modified (i.e., the `SHA-1` hashes don't match), it compares the entire files from `HEAD1` and `HEAD2` line by line using the Longest Common Subsequence (LCS) algorithm, then prints the differences.

---

# **`checkout`**

```bash
vcs checkout <branch-name>
```

```text
      (branch) (main)
                O
         O------O
         O      O
         O      O<---here
         O
         O<----to here.(retreive all the staging area)
```

- This will switch to `branch-name` and discard any uncommitted changes.

![img22](screenshots/checkout/checkout_1.png)
![img23](screenshots/checkout/checkout_2.png)
![img24](screenshots/checkout/checkout_3.png)

```bash
vcs checkout <commit-hash>
```

```text
      (branch) (main)
                O
         O------O
         O      O
         O      O<---here
         O
         O<----to here
         O
         O

      (branch) (main)
                O
                O<---to here
                O
         O------O
         O      O
         O      O<---here
         O
         O
```

- This will switch to the specified `commit-hash`, discard any uncommitted changes, and place `HEAD` in a detached state.

![img24](screenshots/checkout/checkout_3.png)
![img25](screenshots/checkout/checkout_4.png)
![img26](screenshots/checkout/checkout_5.png)

```bash
vcs checkout -b <branch-name>
```

```text
      (branch) (main)
                 O
                 O
                 O
         O-------O<---here
         ^
         |
        to here.
```

- This will create a new branch at the current branch's `HEAD`, switch to it, and discard any uncommitted changes.

![img27](screenshots/checkout/checkout_6.png)

```bash
vcs checkout -b <branch-name> <commit-hash-from-any-branch>
```

```text
		(branch)  (main)
					O
			O-------O
			^       O
			|       O
			to here O
					O<----here 
```

- This will create a new branch at the `commit-hash`, switch to it, and discard any uncommitted changes.

![img28](screenshots/checkout/checkout_7.png)

---

### &#10140; **How It Works**

First, go to the specified commit hash, retrieve the state of that version, and update the .vcs/index (staging area) with its contents. Then, remove the current working directory contents and replace them with the version from that commit. Finally, update .vcs/HEAD to reflect the branch switch or detached HEAD state.

---

# **`merge`**

```bash
vcs merge <branch-name>
```

- This will merge two commits. 

![img29](screenshots/merge/merge_1.png)
![img30](screenshots/merge/merge_2.png)
![img31](screenshots/merge/merge_3.png)

---

### &#10140; **How It Works**

- First, go to the `HEAD` of `branch-name` and get its `commit-hash`. Then, compare it with the `current branch`.
- If a file exists in `branch-name` but not in the `current branch`, it is copied to the `current branch`.
- If a file exists in both branches and is identical, it is ignored.
- If a file exists in both branches but has different content, a `conflict` occurs. The conflicting files are merged into one, and you must manually resolve the conflicts.

---

# **`reset`**

- My last commit was corrupted, incorrect, or Bad, so I want to discard it.

![img32](screenshots/reset/reset_1.png)

```bash
vcs reset --mixed <commit-hash>
```

- This will erase the commits after the specified `commit-hash` and revert the repository to the state of that commit. The staging area is also cleared and restored to match the state of commit-hash. The working directory content remains unchanged.

![img33](screenshots/reset/reset_2.png)
![img34](screenshots/reset/reset_3.png)
![img35](screenshots/reset/reset_4.png)

```bash
vcs reset --soft <commit-hash>
```

- This will erase the commits after the specified `commit-hash` and revert the repository to the state of that commit. The staging area is not cleared and remains unchanged. The working directory content also remains unchanged.

![img36](screenshots/reset/reset_5.png)
![img37](screenshots/reset/reset_6.png)
![img38](screenshots/reset/reset_7.png)

```bash
vcs reset --hard <commit-hash>
```

- This will erase the commits after the specified `commit-hash` and revert the repository to the state of that commit. The staging area is also cleared and restored to match the state of the `commit-hash`. The working directory is updated to reflect that version, and all other changes are removed.

![img39](screenshots/reset/reset_8.png)
![img40](screenshots/reset/reset_9.png)
![img41](screenshots/reset/reset_10.png)

---

### &#10140; **How It Works**

- First, go to the `HEAD` of `branch-name` and get its `commit-hash`. Then, compare it with the `current branch`.
- If a file exists in `branch-name` but not in the `current branch`, it is copied to the `current branch`.
- If a file exists in both branches and is identical, it is ignored.
- If a file exists in both branches but has different content, a `conflict` occurs. The conflicting files are merged into one, and you must manually resolve the conflicts.

---

# **`stash`**

```bash
vcs stash -m <message>
```

- The current working directory is changed to the previous commit state, and all changes are stored in the `stash` along with staging area. The stash is essentially a branch, and with each `stash` operation, a new commit is added to the `stash` branch.

![img42](screenshots/stash/stash_1.png)
![img43](screenshots/stash/stash_2.png)
![img44](screenshots/stash/stash_3.png)

```bash
vcs stash list
```

- Lists all the stashed commits. All stash logs are stored in the `.vcs/logs/refs/stash` file.

![img45](screenshots/stash/stash_4.png)

```bash
vcs stash apply <tag>
```

- This command applies the specified `tag` stash to the current working directory. If changes already exist in the same files, merge conflicts may occur, and you will need to resolve them manually.

![img46](screenshots/stash/stash_5.png)
![img47](screenshots/stash/stash_6.png)

```bash
vcs stash pop <tag>
```

- This command applies and removes the specified `tag` stash from the current working directory. If changes already exist in the same files, merge conflicts may occur, and you will need to resolve them manually.

![img48](screenshots/stash/stash_7.png)
![img49](screenshots/stash/stash_8.png)

```bash
vcs stash drop <tag>
```

- This command removes the specified `tag` stash from `.vcs/logs/refs/stash`.

![img50](screenshots/stash/stash_9.png)

```bash
vcs stash show <tag>
```

- This command shows the diff between the specified `tag` and the latest commit of the current branch.

![img51](screenshots/stash/stash_10.png)

### &#10140; **`.vcs/logs/refs/stash` File Format:**

```text
<parent-hash> <commit-hash> <index-file-hash> <branch> <timestamp> commit: <message>
```

---
