# nhcustom2 (WIP)

This program is the second version of nhcustom, a program created to modify
the Team Fortress 2 mod "no hats mod".

**!! NOT finished. while it shouldn't explode in your face, use it at your own risk. !!**

Some improvements have been made, including:

* A more advanced config syntax
* The use of regular expressions (regexes)
* The ability to search and filter paths

The main drawback of this version is its speed: the program is a lot slower than
its predecessor, especially on slow pcs. This is partly due to a lack of
optimization (this will hopefully be fixed at some point) and to some parts of the
program being handled by shell/batch scripts.

Some parts of the program work differently than in the first version, making the
configuration files made for the original program unusable. However, They are
relatively easy to convert to the new syntax.

The program is known to work with Windows 10 and Fedora 34 (Linux). I don't
offer any support with this program, but feel free to let me know if something
doesn't work as expected or if the database contains errors.


## how to use

Do not launch the program `nhcustom2` directly, unless you want to see what
paths will be copied without actually touching your files. To generate a mod
with your parameters, use the `START` shell/batch script corresponding to your
OS.

The `output` directory is deleted each time the program is run. Backup any mod
previously generated if you want to keep them.

Add the parameters you want in the configuration file, then execute the script.
The script **MUST** be executed while being in the same directory.


## the database

The database is now in the .csv format, making it usable with spreadsheet
programs. It should contain only ASCII characters.


## the configuration file

A file named 'config.txt' needs to be in the same folder as the program. It's the
file containing the script used by the program. Lines beginning with a
pound sign (`#`) are taken as comments and not parsed. It should contain only
ASCII characters.

The **first** line of the file MUST contain either the word (called flag) `remove`
or `keep`. This informs the program wether the hats the file specifies must be
removed from the game, or kept in with everything else removed.

If you plan to remove a lot of cosmetics, I recommand using the `keep` flag. The
database doesn't cover all the cosmetics (like medals) and probably contains
errors. the `keep` flag makes the program work with paths from the input
directory instead, which greatly reduces the risk of encountering unwanted
cosmetics in game.

The syntax of the configuration file is the following:

```
header:pattern
```

With `header` being the type of the argument and `pattern` being the regular
expression that will be used to parse the database.

By default, the values that can go in place of `header` are the following:

* `hat`
* `update`
* `equip` for an equip region
* `class`
* `date`
* `path`

Those values are taken from the first line of the database. One could modify them
or add extra fields, assuming that any new fields are added *in every line* of the
database.

`path` is a new addition. It lets the user specify  paths, which coupled with the new
syntax can for example allow to show/remove specific styles of cosmetics.

`hat`, `update`, `equip`, `class` or `path` can take any word or string, as long as something
in the database matches it. The `date` header, however, **must** be written with
care:

```
date:2021
#will expand to 2021-01-01/2021-12-31
date:2021-05
#will expand to 2021-05-01/2021-12-31
date:2021-05-06
#will expand to 2021-05-06/2021-12-31
date:2021-05-06/2022
#will expand to 2021-05-06/2022-12-31
```

writing

```
date:2021/2022
```

will **NOT** work, as opposed to the previous program.

This is due to a limitation of the program. As a rule of thumb, always write
dates entirely to avoid confusion.

You can also "stack" statements one after the other, by separating them with
a colon `:`. Doing this allows to filter out results found in the first statement.
For example, doing this:

```
Update:Scream Fortress 2020:class:!Soldier:class:!Scout
```

Will find all the hats from the Scream Fortress 2020 update, except the ones
that can be worn by the Scout or the Soldier, and will not affect the hats found
in the previous lines.

The program is now case-insensitive, there is now no need to open the wiki to
get the name of the cosmetic exactly right. (This is only true for the pattern, the
header is case-sensitive.)

*The syntax can be a bit cryptic, I encourage to take a look at the end of the
file where some examples are given, after reading the other points.*


## the exception (`!`) flag

Like in the previous program, you can decide wether or not the pattern to search
is an exception or not. For example,

```
date:2007
hat:!Fancy Fedora
```

will remove every hat from 2007, **except** the Fancy Fedora. With the new
syntax, this statement could also have been written like this:

```
date:2007:hat:!Fancy Fedora
```

## regular expressions (asterisks replacement)

The original program had a concept of "asterisks" (`*`) to select, for example, all
the hats that could be worn by the Scout alone or by the Scout and other classes
as well. This has been removed, and has been replaced by the ability to use
regexes instead.

This brings some differences. For example, writing:

```
class:Scout
```

will match **all** the hats that can be worn by the Scout, instead of the hats that
can be worn only by the scout like previously. To achieve the same result, one
must type instead:

```
class:^Scout$
```

For people experienced with regexes, please note that the regex implementation
used by this program isn't the most feature-complete one, which could cause
some "modern" regexes to fail. The standard used is the [POSIX Extended Regular
Syntax](https://en.wikipedia.org/wiki/Regular_expression#POSIX_basic_and_extended).

Also, there is currently no way of writing regexes with colons in it, or that begin with a `!`.

Another important point is that regexes **cannot** be used with the `date` header.


## the `input` and `output` folders

The `input` folder is where must be an **uncompiled** version of the
no-hats-mod. If done correctly, the path should start with the following:
`input/models/...`

The program will refuse to run if there isn't an `input` folder.

After the program is run, the resulting mod will be placed in the `output` folder,
and is ready to be compiled into a .vpk file by the program of your choice. Like I
wrote previously, the `output` folder is wiped at every start of the program.


## Compiling

On Linux, simply use the makefile in the repository.

If you want to compile this program for Windows 10, you'll have to install
[MSYS2](https://msys2.org/) and use mingw64 when executing the makefile.

This program has only been tested with GCC. It might compile on other
OSes, but I haven't checked.


## Examples

Below are some example of what could be entered in the configuration file.

```
#find hats that can be worn by mutiple classes but that are not all-class
class:.*:class:!All classes

#deselect hats that came out between 2008 and march 1st, 2013, except
#if they can be worn by the soldier (along with other classes)
date:!2008-01-01/2013-3-1:class:!Soldier

#find all the hats containing the string "aaa"
hat:aaa

#find only the 1st style of the Millennial Mercenary
hat:millennial mercenary:path:!style

#find every path with the word "scout" in them
path:.*scout.*

#find only the first style of the Foppish Physician
hat:foppish physician:path:!necktie

```
