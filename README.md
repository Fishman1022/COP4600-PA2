# Concurrent Hash Table Project

## Compiling

Ensure a "commands.txt" file exists in the project directory.

> make all
> ./chash

## AI Usage Disclosure

### Group Members
- Michael Kelly
- Broc Weselmann
- Youssef Amarir
- Caleb Evans
- Adrian Holguin

#### Michael Kelly – AI Usage

AI Link: https://chatgpt.com/share/6920c252-2f34-8003-9d45-ae803b422d68

I used chatgpt to write up a threadRelease function, but I found that I could have written it myself later down the line. The AI kept on trying to release all the locks when that wasn’t the desired output. It also tried using mutex but the team was already using a read and write locks. I gave the prompt what role I was and gave the instructions of the roles that would interact with mine such as the instructions for the commands and their priorities. I also provided some of the sample output from the assignment. 

#### Caleb Evans – AI Usage

AI Link: https://chatgpt.com/share/6920dbe2-8b3c-8009-b070-4733176dacd6

I only used ChatGPT just to kinda to get a better handle on the overall layout of the project. It got switched up also i had to check like how we should split things up between parsing the commands, the hash table functions, and the threading parts. I also used it here and there when I ran into confusing error messages or when I needed a quick sanity check on how certain thread functions works. After I had the idea of the structure i kinda figured it out, I wrote the actual code myself. I handled a lot of the debugging and testing on my own too. I didn’t copy any AI code too much into sthe final project it was just something I used to clear things up and make sure I was on the right track pretty much.

#### Adrian Holguin – AI Usage

AI Link:  https://gemini.google.com/share/f592710a59fd
AI Link:  https://chatgpt.com/share/6920a82c-7aa4-800c-a474-81af872a0bcb 

ChatGPT was first used to outline a plan for the assignment's design and to create a starting point for the rest of the group members to work off. I started off first by using chatGPT to make the hash table structure, implement creation fucntions, as well as setting up a github for other members to use as an uploading platform. The prompts I used generally asked for similar instructions as in the assignment, occasionally providing additional info by way of sample files or other research I could find on the internet. Frequently, I would ask chatGPT to make sure specific instructions were followed, such as always following the given command structure, making sure it followed every command, and also making sure paramaters such as UPDATE made sure to use a key and a priority, as well as an updated salary float to keep things consistent. If an error or inconsistency occurred, sometimes I would paste the error into chatGPT and ask it to find a solution. Oftentimes, however, I ended up fixing most errors myself.

Gemini was also a tool I used to finish up the rest of the project. I mostly used gemini to update the parser given to me by Caleb, as well as to modularize all the final code given by the rest of the group. The group did a great job, but I thought it would be good to modualrize and make different source files to keep things comparmentalized. Gemini was also used to consult for different issues, such as commands being issued earlier than anticipated, and for solutions to the race conditions our code frequently ran into (The solution ended up being to use a long delay timer). Finally, making sure our output was correctly formatted and followed the right rules, as well as making sure our log logic worked was handled by Gemini as well. Comments, file structure, and input/output reading were handled by Gemini, as well as the finalization of the hashTable logic. I'd prompt Gemini about errors, advice on specific issues, asking for specific functionality (like the log output and the sample output), and making sure things were consistent. Overall, AI was very instrumental to the assignment working. Gemini was also finally used to make the make file, which I had to edit a couple times with Gemini to ensure no new text was added, and so the grader could read the final results. This also meant makin changes to chash to ensure pausing after final output was a possibility.

#### Broc Weselmann – AI Usage

AI Link: https://gemini.google.com/app/0e456a7967bfef7c

Gemini pretty easily one-shot my contribution to the project, implementing hashInsert and hashDelete without any particular prompt engineering. I was confused about the priority system, but Gemini clarified it and showed through the conversation. I wrote the code from the conversation myself and did not copy & paste the code. This was because I needed to remove some unnecessary print statements that don't show up in the provided output table for the functions, and to generally program. Gemini also did not implement the jenkins_one_at_a_time_hash function, but that was easily copied and repurposed from Wikipedia.

#### Youssef Amarir – AI Usage

AI Link: https://gemini.google.com/share/c130740e02d8

Pretty much I just used Gemini to just understand what i was doing and what the assignment was actually asking for at first I had to ask it a few times along the way because the assignment changed a few times I also to make sure I wasn’t overthinking the hash search and hash print. Basically, I kinda just used it to double check some of the concepts I didn't get and so I wasn’t missing anything. Everything I wrote though in the final submission for the most part was my own code pretty much Gemini just helped me understand the rules of the assignment and clear up the confusing parts.
