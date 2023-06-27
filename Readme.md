# C Backend Framework (CBack)
CBack is a C library for building basic concurrent RESTful web servers with limited functionalities limited to classroom level applications.
Not recommended for high performance applications. Solely written for the purpose of attempting to 'easily' write a backend in C without sweating too much!

**Features:**
- HTTP 1.1 compatible request parser
- RESTful oriented interface
- Handler creation for endpoints
- Template rendering for endpoints
- Custom dictionary module for data handling
- Equiped with form data, query parameters, JSON parsers
- Implementation is HTTP 1.1 compliant
- Multiple threading models
- Support for cookie based authentication
- Support for CORS and CSRF

## Table of Contents
* [Introduction](#introduction)
* [Requirements](#requirements)
* [Building](#building)
* [Getting Started](#getting-started)
* [Structures definition](#structures-and-classes-type-definition)
* [Create and work with a webserver](#create-and-work-with-a-webserver)
* [Parsing requests](#parsing-requests)
* [Building responses to requests](#building-responses-to-requests)
* [Authentication](#authentication)
* [Other Examples](#other-examples)


## Introduction
CBack is meant to constitute an easy system to build university level HTTP servers with REST fashion in C.
The mission of this library is to support all possible HTTP features directly and with a simple semantic allowing then the user to concentrate only on his application and not on HTTP request handling details. With this framework, you can enter the club of developers who wrote RESTful HTTP server in C, but by sweating a lot lesser!

CBack is able to decode certain body formats and automatically format them in custom dictionary fashion. This is true for query arguments and for *POST* and *PUT* requests bodies if *application/x-www-form-urlencoded* header are passed.

[Back to TOC](#table-of-contents)

