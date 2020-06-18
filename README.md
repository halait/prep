# Prep
A simple language agnostic preprocessor.


## Directives
### Include
The `include` directive includes the contents of the specified file into the output file. The specified file is also preprocessed before inclusion. The indentation in the line of the directive is inserted before all subsequent lines that are included.
#### Example
Suppose `tail.txt` is:
```
I am multi-line text to be
inlcuded in the output file.
```
And suppose `in.txt` is:
```
  Lorem ipsum dolor sit amet, consectetur adipiscing elit, sed do
  eiusmod tempor incididunt ut labore et dolore magna aliqua.
    {{ include tail.txt }}
```
If both are placed in the same directory and Prep is invoked in that directory with the command:
```
prep /Fe:out.txt in.txt
```
It will write a file called `out.txt` with the following content:
```
  Lorem ipsum dolor sit amet, consectetur adipiscing elit, sed do
  eiusmod tempor incididunt ut labore et dolore magna aliqua.
    I am multi-line text to be
    inlcuded in the output file.
```
#### Syntax
`{{ include path }}`
##### Parameter
`path`: The path of the file to be included. Throws `invalid_argument` if path is empty. Throws `runtime_error` if file specified cannot be read.

### Define
The `define` directive replaces all instances of the defined variable with its value.
#### Example
Suppose `in.txt` is:
```
{{ define foo
Lorem ipsum dolor sit
ametconsectetur adipiscing elit
}}
Stuff before foo stuff after.
    Note indentation is inserted for subsequest lines of foo    
```
If Prep is invoked in the directory containing `in.txt` with the command:
```
prep /Fe:out.txt in.txt
```
It will write a file called `out.txt` with the following content:
```
Stuff before Lorem ipsum dolor sit
ametconsectetur adipiscing elit stuff after.
    Note indentation is inserted for subsequest lines of Lorem ipsum dolor sit
    ametconsectetur adipiscing elit    
```
#### Syntax
`{{ define variable value }}`
##### Parameters
`variable`: The variable to be defined. It cannot contain white-space. Throws `invalid_argument` if empty.<br>
`value`: The value given to the variable. It can be empty.


## Install
Download the source file and compile it with your favourite C++ compiler. The executable should be placed inside a directory in your PATH.


## Tutorial
### Including files
Suppose your making a website with multiple pages that contain a duplicate header and footer. You can avoid copying and pasting the header and footer into each page by using Prep. First write the header and footer into their own seperate files called `header.html` and `footer.html` respectively in a directory called `example`. This is the contents of `header.html`:
```html

```
And this is the contents of `footer.html`:
```html
		<div id="footer">
			<address>
				Email Address: <a href="mailto:no_reply@fake.foo">no_reply@fake.foo</a><br>
				Phone Number: <a href="tel:+15555555555">1-555-555-5555</a>
			</address>
		</div>
	</body>
</html>
```
In the same directory write another file called `index-src.html` with the following content:
```html
{{ include header.html }}
		<h1>My landing page</h1>
    Lorem ipsum dolor sit amet, consectetur adipiscing elit, sed do
    eiusmod tempor incididunt ut labore et dolore magna aliqua. Ut
    enim ad minim veniam, quis nostrud exercitation ullamco laboris
    nisi ut aliquip ex ea commodo consequat.
{{ include footer.html }}
```
In `index-src.html` we use `include` preprocessor directives that Prep interprets to include the contents of `header.html` and `footer.html` into the output file that it creates. Now run the following command in your command-line interperter inside the `example` directory.
```
prep /Fe:index.html index-src.html
```
The `/Fe` option is used to specify the name of the output file and `index-src.html` is the name of the input file. This will write a file called `index.html` in the same directory with the contents of `index-src.html` except the `include` preprocossor directives will be replaced with the contents of the file specified. This will be the contents of `index.html`:
```html
<!doctype html>
<html>
	<head>
		<meta name="viewport" content="width=device-width, initial-scale=1">
		<link rel="stylesheet" href="style.css">
		<link rel="icon" type="image/png" href="img/icon.png">
		<title>My title</title>
		<meta name="description" content="My meta description">
	</head>
	<body>
		<div id="nav-bar">
			<a href="index.html"><img id="logo" src="/img/logo.png" alt="Logo"></a>
			<ul id="nav-links">
				<li><a href="explore.html">Explore</a></li>
				<li><a href="contact.html">Contact</a></li>
			</ul>
		</div>
		<h1>My landing page</h1>
    Lorem ipsum dolor sit amet, consectetur adipiscing elit, sed do
    eiusmod tempor incididunt ut labore et dolore magna aliqua. Ut
    enim ad minim veniam, quis nostrud exercitation ullamco laboris
    nisi ut aliquip ex ea commodo consequat.
		<div id="footer">
			<address>
				Email Address: <a href="mailto:no_reply@fake.foo">no_reply@fake.foo</a><br>
				Phone Number: <a href="tel:+15555555555">1-555-555-5555</a>
			</address>
		</div>
	</body>
</html>
```

If you want to discuss or use this repository please contact me.
