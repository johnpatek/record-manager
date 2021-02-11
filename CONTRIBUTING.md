# Contributing

When contributing to this repository, please first discuss the change you wish to make via issue,
[email](mailto:johnpatek2@gmail.com), or any other method with the owners of this repository before making a change.

## Pull Request Process

1. Create a pull request and make sure @johnpatek is added as a reviewer.
2. Wait for the reviewer to finish adding comments.
3. Address any comments and make any changes requested by reviewers.
4. Merge the completed pull request or notify one of the reviewers when it is ready to be merged.

## Best Practices

Please use the following guidelines to ensure that pull requests can be quickly reviewed and accepted.

+ Keep each request limited to a single change or feature. If your changes require a large overhaul of
the existing code, discuss via [email](mailto:johnpatek2@gmail.com).
+ Add any necessary dependencies to the `external` directory as a submodule. If they are meant to replace
an existing dependency, please remove the existing dependency and its submodule.
+ Verify that the CI pipeline is not broken. This can be checked by either the build status on the READ_RECORDME or
by checking the pipelines on travis-ci.
+ In general, bug fixes will only require changes to the source and header files and can be tested using
the existing unit tests, whereas features will often require changes to the unit tests.
