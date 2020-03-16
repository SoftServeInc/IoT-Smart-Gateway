# IoT Smart Gateway

## Development process 

### Branching strategy

 - **master** - only for stable releases. Branch is protected from push. Updates only as merges from release branch
 - **development** - branch for development version. Protected from push. Updates only as Pull Requests from feature branches
 - **feature** - feature branches. Is created for some feature and then merges to development brach as Pull Request
 - **release** - is like *merge-development-to-master*. Every release updates increments product version

### Development workflow sample

**Example:** Developer creates feature branch for resolving some http request, named feature/resolve-http. Makes few commits, pushes to feature branch and opens PR. After approval, it merges to development branch.
When team decides to make a new release, teamlead creates release branch. After all checks it merges to master.

