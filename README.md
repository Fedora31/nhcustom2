# nhcustom2

> Note (Jan 2023): v3 is out! The main changes are: A new system called "lists" has been
> introduced, which can simplify configurations or allow to create more complex ones.

This program is the second version of [nhcustom](https://github.com/Fedora31/nhcustom),
a program created to modify the Team Fortress 2 mod "no hats mod". If you plan to modify
it to stop seeing specific hats or handpick those you want to still see, this program makes
doing so easier.

**Note:** A problem can arise when creating a custom version of
"[no-hats-bgum](https://github.com/Fedora31/no-hats-bgum)", where cosmetics appear
clipped inside default headgears. It isn't a problem with this program, but with how
no-hats-bgum is made. See
[this image](https://github.com/Fedora31/no-hats-bgum/blob/master/.github/images/nhm_modify.png)
for more information.

Some improvements have been made, including:

* A more advanced config syntax
* The use of regular expressions (regexes)
* The ability to search and filter paths

Some parts of the program work differently than in the first version, making the
configuration files made for the original program unusable. However, They are
relatively easy to convert to the new syntax.

The program is known to work with Windows 7, 10 and Fedora 36 (Linux). I don't
offer any support with this program, but feel free to let me know if something
doesn't work as expected or if the database contains errors.

The program was created to work with the
[bgum version](https://github.com/Fedora31/no-hats-bgum) of "no hats mod",
but should work with other versions.

If you want the latest version, you should consider building the program from source,
as I won't make new releases for small changes.


## How to use

Add the parameters you want in the configuration file, then execute the program. An `input`
folder must be present next to the executable, or another folder must be specified with
the option `-i`.

The input folder must be filled with the content of
[this zip file](https://github.com/Fedora31/no-hats-bgum/blob/master/nhm_source/mechbgum_no_hats_mod/no_hats_bgum.zip), or any other version of "no hats mod".

Make sure to keep the database and the input folder up do date.


## Arguments

The program accepts the following arguments:

* `-q`, don't print info messages.
* `-p`, print the found paths to stdout.
* `-n`, don't touch to any file (the program has no effect).
* `-i`, specify the input folder. (default: `./input`)
* `-o`, specify the output folder. (default: `./output`)
* `-f`, specify the configuration file. (default: `./config.txt`)
* `-d`, specify the database. (default: `./database.csv`)
* `-s`, specify the separator used in the database. (default: `;`)
* `-h -?`, show the help.


## The database

The database is now in the .csv format, making it usable with spreadsheet
programs. It should contain only ASCII characters and must not possess
any carriage returns in it (the CR in CR+LF line endings), only line feeds.


## The configuration file

A file named 'config.txt' needs to be in the same folder as the program. It's the
file containing the script used by the program. Lines beginning with a
pound sign (`#`) are taken as comments and not parsed. It should contain only
ASCII characters.

The configuration file should contain either the word (called command, previously
flag) `remove` or `keep`. This informs the program whether the hats the file
specifies must be removed from the game, or kept in with everything else removed.

If you plan to remove a lot of cosmetics, I recommand using the `keep` command. The
database doesn't cover all the cosmetics (like medals) and probably contains
errors. the `keep` command makes the program work with paths from the input
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
* `list`

Those values, appart from `list`, are taken from the first line of the database. One could
add extra fields in it, assuming that any new fields are added *in every line* of the database.
Note that headers **cannot** contain a colon (`:`).

`list` is a reserved keyword and so cannot be used in the database (See [Lists](#lists)).

`path` lets the user specify  paths, which coupled with the new syntax can for example allow to
show/remove specific styles of cosmetics.

`hat`, `update`, `equip`, `class` or `path` can take any word or string, as long as something
in the database matches it. The `date` header, however, **must** be written with
care:

```
date:2021
date:2021-05
date:2021-05-06
date:2021-05-06/2022
date:2021-05-06/2022-12
#since v2 (Dec 2022), the dates above are NOT valid.

date:2021-05-06/2022-12-31
#this date is valid.
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

If one wants to write a literal colon that mustn't be interpreted as a separator,
they can escape it with a backslash like so: `\:`. This means that literal backslashes
must also be escaped (`\\`).

*The syntax can be a bit cryptic, I encourage to take a look at the end of the
file where some examples are given after reading the other points.*


## Regular expressions (asterisks replacement)

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

An important point is that regexes **cannot** be used with the `date` and `list` headers.


## The exception (`!`) flag

Like in the previous program, you can decide wether or not the pattern to search
is an exception or not. For example,

```
date:2007
hat:!Fancy Fedora
```

will search every hat from 2007, **except** the Fancy Fedora. With the new
syntax, this statement could also have been written like this:

```
date:2007:hat:!Fancy Fedora
```


## The filter (`}`) flag

Since v2, you can use this new flag to filter out any previous matches that
do not match with the following statement. This is especially useful when you
are interested in a small subset of previous results. For example, if you wanted
to select all the cosmetics that came out in the Smissmas 2022 update for the Medic
and the Pyro, you could write the following:

```
update:Smissmas 2022:class:}^Medic$|^Pyro$
```

Note that flags cannot be used together. If one wants to use a flag character as a
literal character, they can also write a backslash before it (ex: `\!`), but this
is only needed when the character is at the beginning of the string.


## Lists

This is a new addition which came with v3. With the `list` command, one is able to
store results from searches into "lists" which later can be used in other searches.
It was created to allow users to store results from multiple lines with no effect on
previous results.

Example:

```
list foo update:Smissmas 2022:list:!bar
```

This creates a new list called `foo` which contains all the cosmetics from Smissmas
2022, except whichever result was in the already existing list `bar`.

It's also possible to redeclare lists and even reuse the old content to do so:

```
list foo update:Smissmas 2022
list foo list:foo:hat:A Rather Festive Tree
```

With this example, you end up with a list `foo` containing the cosmetics from
Smissmas 2022 and the hat "A Rather Festive Tree" (this example isn't very useful,
it's just for demonstration).

Lists can also achieve advanced configurations that were previously impossible, like
this example:

```
date:2009-01-01/2015-12-31
# ...

list globalfilter class:^Scout$:equip:}^Feet$
list globalfilter class:^Pyro$:equip:}^Back$:list:globalfilter
list:}globalfilter
#only Pyro's backpacks and Scout's shoes remain
```

Note: List names mustn't contain any spaces in them.


## The input and output folders

The input folder is where must be an **uncompiled** version of the
no-hats-mod. If done correctly, the path should start with the following:
`input/models/...`

The program will refuse to run if there isn't an input folder.

After the program is run, the resulting mod will be placed in the output folder,
and is ready to be compiled into a .vpk file by the program of your choice.

As of v2, The output folder is *no longer wiped at each startup*, so old generated
mods won't be deleted. This has been changed with the release of the `-o` option,
as I judged the old behavior to be too dangerous now that the user can specify
their own, potentially wrong or misspelled, output folder.


## Compiling

On Linux, simply use the makefile in the repository.

If you want to compile this program for Windows, you'll have to install
[MSYS2](https://msys2.org/) and use mingw64 when executing the makefile.

This program has only been tested with GCC and glibc. It might compile on other
environments, but I haven't checked.


## Examples

Below are some examples of what could be entered in the configuration file.

```
#find hats that can be worn by mutiple classes but that are not all-class
class:.*\|.*:class:!All classes

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

#find only the third style of the medic cosmetics from Smissmas 2022
update:Smissmas 2022:class:}^Medic$:path:}style3

#create a list containing the Engineer's shirts and all VALVe-made hats for the spy
list mylist class:^Engineer$:equip:}Shirt
list mylist class:^Spy$:equip:}Hat:path:}models/player/items:list:mylist

```
