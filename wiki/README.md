# Bash++

> Bash with classes

```bash
@class Bashpp {
  @public author="Andrew S. Rightenburg"
  @public source="https://github.com/rail5/bashpp"
  @public license="GNU GPL v3"

  @public @method printInfo {
    echo "Bash++ is a superset of Bash that adds support for classes and objects."
    echo "It's meant to be a simple way to add object-orientation to Bash scripts."
    echo "Author: @this.author"
    echo "Source: @this.source"
    echo "License: @this.license"
  }
}

@Bashpp myBashpp
@myBashpp.printInfo
```

## Documentation

 - [Programming in Bash++](language.md)
 - [Using the Bash++ compiler](compiler.md)

## License

![GNU GPL v3](https://www.gnu.org/graphics/gplv3-with-text-136x68.png)

The Bash++ language and compiler are licensed under the [GNU General Public License v3](https://www.gnu.org/licenses/gpl-3.0.html).

This does not affect any code you write in Bash++ -- only the Bash++ language and compiler themselves. You are perfectly free to use Bash++ to write proprietary software, or software under a different license.

## Documentation License

![CC-BY-SA](https://licensebuttons.net/l/by-sa/3.0/88x31.png)

This documentation is licensed under the [Creative Commons Attribution-ShareAlike 4.0 International](https://creativecommons.org/licenses/by-sa/4.0/) license.

**You are free to:**

 - **Share** -- copy and redistribute the material in any medium or format for any purpose, even commercially

 - **Adapt** -- remix, transform, and build upon the material for any purpose, even commercially.

**Under the following terms:**

 - **Attribution** -- You must give [appropriate credit](https://creativecommons.org/licenses/by-sa/4.0/#ref-appropriate-credit), provide a link to the license, and [indicate if changes were made](https://creativecommons.org/licenses/by-sa/4.0/#ref-indicate-changes). You may do so in any reasonable manner, but not in any way that suggests the licensor endorses you or your use.

 - **ShareAlike** -- If you remix, transform, or build upon the material, you must distribute your contributions under the [same license](https://creativecommons.org/licenses/by-sa/4.0/#ref-same-license) as the original.

I also give my personal (though I suppose legally dubious) permission to redistribute this documentation, modified or unmodified, without providing attribution. If there was a Creative Commons ShareAlike license without an attribution clause, I would've selected it. All that matters to me is that other people are given the **same rights** to *your changes* that **you were given** to the *original.*
