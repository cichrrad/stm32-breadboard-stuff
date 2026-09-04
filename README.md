# What is this?

This is a repo of STM32 oriented breadboard-esque/fun projects to incrementally learn and graps the ways of embedded/bare-metal programming, mainly with C. 

I made this for myself to document and be able to look back upon progression from basic things which may *work*, but are *wrong* in the context of safety-critical / embedded developement and such. 

I make things up as I go, but huge inspirations are:

* [Equip Embedded on Youtube](https://www.youtube.com/@EquipEmbedded)

* [hodd._world on Youtube](https://www.youtube.com/@hoff._world)

> Board I use -- STM32 NUCLEO G491RE

## Overview

The way I organize files is dynamic and ongoing, as of now it is:

 1. Projects are in numbered files `xxx_project_name`.
 2. Each project has `README.md` with notes which might explain some things or tie it in to previous project.
 3. `Includes` and `Sources` directories are for includes across multiple projects. Currently it is a bit messy, as I (in my infinite wisdom) decided to split it up into Bare-Metal (BM) and RTOS, because some of my previous code had to be changed, BUT I did not want to break older projects. In `[Includes or Sources]/Common`, there are some utils and also cmsis headers. If some project needs code I have yet to adapt to be generic, it will be in that project directory in `local_source`. If there are demos or stuff such as `.mp4` file to work with, it will be in the project dir in `assets`.
 4. Each project has its own `linker.ld`, `startup.c`, `main.c`, and `Makefile`. Based on the project, these might (will) differ based on what I want to compile + they changed overtime as I realized things here and there. For instance project needing interrupt handlers needs to extend interrupt vector in `startup.c` to accomodate them. Common pattern for `Makefile` is that you build with `make build` and flash with `make flash`.
 5. Projects with numbers less than 200 are reserved for *micro* projects, which are really small and often variations of few basic apps (blinky). These mainly serve for the incremental improvement -- ultimately doing the same thing or task (blinky), but better or in different ways to explore and improve stuff.
 6. Projects with number more than or 200 are reserved for more complex projects, combining different things together.
 ## Environment

 To replicate my working conditions (SW-wise, for HW you have to source the board etc..), You can use devcontainers and vscode. `.devcontainer` dir is included, and working in it is exactly what I do. For additional dependencies specific for a project (for instance, some python modules for `201_bad_apple`), `README.md` and `Makefile` comments for that specific project should guide well enough, provided I don't get lazy.

---

 > Cheers! -RC