# nhcustom2

This program is the second version of nhcustom, a program created to modify
the Team Fortress 2 mod "no hats mod".

Some improvements have been made, including:

* A more developped config syntax
* The use of regular expressions (regexes)

Some parts of the program work differently than in the first version, making the
configuration files made for the original program unusable. However, They are
relatively easy to convert to the new syntax.

The program is known to work with Windows 10 and Fedora (Linux). I don't
offer any support with this program, but feel free to let me know if something
doesn't work as expected or if the database contains errors.


## the database

The database is now in the .csv format, making it usable with spreadsheet
programs. It should contain only ASCII characters.


## the configuration file

A file named 'config.txt' needs to be in the same folder as the program. It's the
file containing the script used by the program. Lines beginning with a
pound sign (`#`) are taken as comments and not parsed. It should contain only
ASCII characters.

The **first** line of the file MUST contain either the word `remove` or `keep`.
This informs the program wether the hats the file specifies must be removed
from the game, or kept in with everything else removed.

The syntax of the file is the following:

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

Those values are taken from the first line of the database. One could modify them
or add extra fields, assuming that any new fields are added *in every line* of the
database.

`hat`, `update`, `equip` or `class` can take any word or string, as long as something
in the database matches it. The `date` header, however, **must** be written like
so:

```
date:2021
#or
date:2021-05
#or
date:2021-05-05
#or
date:2021-05-05/2022
#or
date:2021-05-05/2022-05
#or
date:2021-05-05/2022-05-05

#NOT
date:2021-05/2022-05-05
#(its missing the days of the first date)
#...or any sort of notation
```

This is due to a limitation of the program. I however think it's not terribly
difficult to follow those rules.

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
some advanced regexes to fail. Also, there is currently no way of writing
regexes with colons in it, or that begin with a `!`.

Another important point is that regexes **cannot** be used with the `date`
header.


## the `input` and `output` folders

The `input` folder is where must be an **uncompiled** version of the
no-hats-mod. If done correctly, the path should start with the following:
`input/models/...`

After the program is run, the resulting mod will be placed in the `output` folder,
and is ready to be compiled into a .vpk file by the program of your choice.


## Examples

Below are some example of the syntax used by the configuration file.

```
#find hats that can be worn by mutiple classes but that are not all-class
class:.*:class:!All classes

#deselect hats that came out between 2008 and march 1st, 2013, except
#if they can be worn by the soldier (along with other classes)
date:!2008-2013-3-1:class:!Soldier

#finds all the hats containing the string "aaa"
hat:aaa

```
