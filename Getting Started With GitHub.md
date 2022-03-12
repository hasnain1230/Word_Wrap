# Getting Started With GitHub Repositories
Okay, okay... I know... Learning how to setup a first time Git repo and learning how to use Git can be / is super overwhelming, but it works really good at a version control system, so just bear with me. It's not that bad. I'll outline how to get started with Git for any project and such. These are just the basic steps that should get you up and running. If something goes wrong, you can Google it or ask me. Whichever works for you, lol.

## Initializing Your Git Repository
So, I have already made the remote Git repository. That is this one, the one you are currently looking at. Okay, now, log into your iLab through VSCode and make your project directory. This can be wherever you want. This is where all your project files and code will be for this project. Once you make that directory, navigate there with the terminal using the following command:
```
cd /path/to/folder
```
Where `path/to/folder` is the directory path to where your project files are. For example, if your project directory is in `/common/home/{net_id}/Desktop/programming_assignment`, you would enter `cd /common/home/{net_id}/Desktop/programming_assignment`.

Once you successfully navigate to the right folder, enter the following commands. This will do the first time setup for your Git repository. Once you do this, you will not need to do this again until or unless you are setting up a new repository elsewhere. In other words, you only do this one time for each repository you are working on. 

Start off by initializing your Git repository and the `main` branch by entering the following command:
```
git init -b main
```

This will initialize Git in that repository and establish a `main` branch.

Now, we need to add the remote repository to our local Git repository. We do this with the following command:
```
git remote add origin https://github.com/hasnain1230/Word_Wrap.git
```
**Please note, the last part of the URL is the repository name followed by .git. You can find this link in any repository by clicking the green `code` button, then copying the `HTTPS` link and pasting that in as the remote after `origin` in the command above.**

This should set up the remote repository with the `main` branch. Congrats. You are all setup and good to go. If it asks you for a password, you need to input your GitHub authorization token that you should have saved and/or need to regenerate. 

## Committing To The Repository
Now that you have your repository setup, we can start coding. First, always pull all the most recent changes from the `main` branch. This may change if you have another active working branch like `dev`. Then you would pull from `dev`. To get started though first, we pull from `main`.
```
git pull origin main
```

**Please note: do this before you make any changes.**

Now your local branch is up to date with the remote branch. Now, you can start making changes. If you are going to start without on a new feature, first create a new separate local branch to work on said feature. You can do this by:
```
git checkout -b <branch_name>
```
Say you want to work on the `enter_button` feature. You would enter:
```
git checkout -b enter_button
```

Now, you are in your branch and you can start making your changes. Once you are done and ready to commit them, you can commit your new files by doing the following:
```
git add .
```

This will add all your new and changed files to the staging area.

Then, you need to commit these changes locally. You can do this by entering the following:
```
git commit -m <message (details about commit)>
```

You can also add additional and longer messages in the description of the commit by doing the following:
```
git commit -m <message (title detail about commit)> -m <More Details about commit (description of commit)>
```
This is optional. You can write your more detailed message in your pull request if you'd like. It doesn't really matter as long as it is clear what you are committing. You also do not need to commit only finished and polished features. You can also commit stuff that is a work in progress, but you just want to commit it now so that you don't have one massive, giant and confusing commit at the end. Having a case like that is bad.

Now, push your changes out to GitHub.
```
git push origin <branch_name>
```
(This should be the branch name you committed your changes to.)

If all goes well, you can now go to the online GitHub repository and view your most recent commit. 

From here, you can go to the branch, make a Pull Request to merge with any other branch that you please. 

(Please keep in mind that for Systems Programming, we are reviewing each other's pull requests, so don't approve your own pull request unless it is a minor bug that you are fixing (in which case, go for it!) )
