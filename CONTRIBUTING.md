
#### Table Of Contents


[Code of Conduct](#code-of-conduct)

[How Can I Contribute?](#how-can-i-contribute)
  * [Reporting Vulnerabilities](#reporting-vulnerabilities)
  * [Reporting Bugs](#reporting-bugs)
  * [Suggesting Enhancements](#suggesting-enhancements)
  * [Pull Requests](#pull-requests)
  * [Documentation Maintenance](#documentation-maintenance)

[Styleguides](#styleguides)
  * [Git Commit Messages](#git-commit-messages)
  * [Google C++ Styleguide](https://google.github.io/styleguide/cppguide.html)
  * [Documentation Styleguide](#documentation-styleguide)

[Contribution Guide Thanks](#contribution-guide-thanks)

## Code of Conduct

1. Be nice and friendly
2. Avoid any kind of abuses or strong language.
3. Everything that You want to deliver to our community should be in well structured and polite form.
4. Following our guidelines is mandatory thing for having clear understanding of our processes.


## How Can I Contribute?
There are several ways to make contribution:
* [Reporting Vulnerabilities](#reporting-vulnerabilities)
* [Reporting Bugs](#reporting-bugs)
* [Suggesting Enhancements](#suggesting-enhancements)
* [Documentation Maintenance](#documentation-maintenance)


### Reporting Vulnerabilities

If You discovered the vulnerability:
  1. DO NOT create an issue on GitHub, even with the pull request.
     Help us avoid publicly known security issues.
  2. Please, send us a detailed vulnerability report on the following e-mail:
But, if You have a fix that You want to propose, send us an e-mail with:
  1. Detailed vulnerability report with link to Your repository.
  2. List of commits that are created for vulnerability fix (required only if Your repository contain changes that are not related to vulnerability fix).

Report template and example,
 [just fill and send](TEMPLATES/bug_report.md).


### Reporting Bugs

Bug severity explanation:

1. SUASS (Stand up and start scream) - Vulnerability
2. Heavy -  Gateway crash
3. Normal - Bug that avoids proper Gateway working routine (Some Gateway's service stopped working, etc...)
4. Minor -  There is no discomfort or relatively small discomfort in usage.
5. Cosmetic - Code style issues/ bad formatted text, etc..
6. Don't know

In any case, if You are not sure what severity You should choose, choose severity 6.

Firstly, each bug is being tracked as a GitHub issue.
Before creating a bug report, **please check repository issue list** as You might find out that You don't need to create one.
> **Note:** If You find a **Closed** issue that seems like it is the same thing that You're experiencing, open a new issue and include a link to the original issue in the body of Your new one.

At this point, we determined that Your's issue is unique, and we should create a very detailed and descriptive bug report.

So, You need to create an issue on our repository and provide the following information by filling in [the template](TEMPLATES/bug_report.md).


Explain the problem and include additional details to help maintainers reproduce the problem:

* **Use a clear and descriptive title** for the issue to identify the problem.
* **Describe the exact steps which reproduce the problem** in as many details as possible.
For example, start by explaining how You configured Gateway, e.g. which command exactly You used in the terminal, or how You configured network-related Linux services. When listing steps, **don't just say what You did but explain how You did it**. For example, if You changed Gateway configuration, explain what exactly You have changed in comparison with previously working case, also, specify what You are trying to achieve, maybe there could be suggested confidently working scenario.
* **Describe the behaviour You observed after following the steps** and point out what exactly is the problem with that behaviour.
* **Explain which behaviour You expected to see instead and why.**
* **If makes sense, include screenshots and animated GIFs** which show You following the described steps and clearly demonstrate the problem.

Provide more context by answering these questions:
* **Did the problem start happening recently** (e.g. after updating to a new version of Gateway) or was this always a problem?
* If the problem started happening recently, **can You reproduce the problem in an older version of Gateway?** What's the most recent version in which the problem doesn't happen?


* **Can You reliably reproduce the issue?** If not, provide details about how often the problem happens and under which conditions it normally happens.

Include details about Your configuration and environment:

* **Which released version are You using?** You could find it in a special file.
* **What is the name and version of the OS You are using**?

### Suggesting Enhancements

Before creating enhancement suggestions, please check the repository issue list as You might find out that You don't need to create one. When You are creating an enhancement suggestion, please include as many details as possible. Fill in [the template](TEMPLATES/feature_request.md), including the steps that You imagine You would take if the feature You're requesting existed.
Explain the feature and include additional details:

* **Use a clear and descriptive title** for the issue to quickly figure out the idea of suggestion.
* **Provide a step-by-step description of the suggested enhancement** in as many details as possible.
* **Describe the current behaviour** and **explain which behaviour You expected to see instead** and why.
* **Explain why this enhancement would be useful**
* **List some other applications where this enhancement exists.**
* **Specify which version of Gateway You're using.** You can get a version from special config file
* **Specify the name and version of the OS You are using.**

### Pull Requests

Here the checklist of what You need to create a successful pull request:
1. Point issue on the repository
2. Descriptive commit messages. [Checkout the guide](#git-commit-messages)
3. In case You have changed the behaviour of some feature/ widely used function, etc....
     1. Provide a justification for this change.
     2. Change according documentation that listed in documentation file in the folder with code. Also, read [documentation guide](#documentation-maintenance)
4. There is no need to assign reviewers to Your PR, we will do it for You.
5. Pay attention to reviewers suggestions and comments.
6. Pull request will merged if and only if there is 3 approvals on it
7. Always make clear build before Pull Request submitting.
8. Always reply on PR Code review questions and suggestions.
9. Run all possible tests that You are able to run in Your environment before submitting PR.
10. In case some tests was not able to run in Your environment, specify this fact, and also some details about environment in PR comments.

### Documentation Maintenance

In case You have made any behaviour change, You must change documentation file located in each directory.
[Checkout out the documentation guide](#documentation-styleguide).

### Styleguides

* [Google C++ styleguide](https://google.github.io/styleguide/cppguide.html)
* [Documentation Styleguide](#documentation-styleguide)
* [Git Commit Messages](#git-commit-messages)

### Git Commit Messages

* [Use this guide to write acceptable commit message](https://wiki.openstack.org/wiki/GitCommitMessages)

### Documentation Styleguide



### Additional notes


#### Gratitude

Thanks to Atom Contribution Guide for great example.
